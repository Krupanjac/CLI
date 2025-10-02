#include <string>
#include <iostream>
#include <fstream>
#include <cctype>
#include <stdexcept>

#include "Stream.h"

// ---------------- Stream (manager) ----------------
Stream* Stream::instance() {
    static Stream s;
    return &s;
}

Stream::Stream() : first(nullptr) {}
Stream::~Stream() { clear(); }

void Stream::insert(InputStream* node) {
    if (!node) return;
    if (!first) { first = node; return; }
    InputStream* cur = first;
    while (cur->getNext()) cur = cur->getNext();
    cur->setNext(node);
}

void Stream::setFirst(InputStream* f) { first = f; }
InputStream* Stream::getFirst() const { return first; }

void Stream::clear() {
    InputStream* cur = first;
    while (cur) {
        InputStream* nxt = cur->getNext();
        delete cur;
        cur = nxt;
    }
    first = nullptr;
    pipeline.clear();
}

void Stream::split(const std::string& line) {
    pipeline.clear();
    std::string segment;
    for (char c : line) {
        if (c == '|') {
            if (!segment.empty()) { pipeline.push_back(segment); segment.clear(); }
        } else {
            segment += c;
        }
    }
    if (!segment.empty()) pipeline.push_back(segment);
}

static bool isTxtFileCandidate(const std::string& arg) {
    return arg.size() >= 4 && arg.front() != '"' && arg.back() != '"' && arg.substr(arg.size() - 4) == ".txt";
}

static void trimRight(std::string& s) {
    while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
}
static void trimLeft(std::string& s) {
    size_t i = 0; while (i < s.size() && std::isspace(static_cast<unsigned char>(s[i]))) ++i; s.erase(0, i);
}
static void trim(std::string& s){ trimRight(s); trimLeft(s);}    

// Extract trailing redirections and support forms without spaces like "<file" or ">>file"
static void parseRedirections(std::string& raw, std::string& inFile, std::string& outFile, bool& appendOut) {
    const char GT = '>';
    const char LT = '<';

    inFile.clear(); outFile.clear(); appendOut = false;
    // Allow both in any order; consume from the end repeatedly
    bool changed = true;
    while (changed) {
        changed = false;
        trimRight(raw);
        if (raw.empty()) break;

        int j = static_cast<int>(raw.size()) - 1;
        // skip trailing whitespace
        while (j >= 0 && std::isspace(static_cast<unsigned char>(raw[j]))) --j;
        if (j < 0) break;

        // move back to start of filename token; stop at whitespace or a redirection operator
        int end = j;
        while (j >= 0 && !std::isspace(static_cast<unsigned char>(raw[j])) && raw[j] != LT && raw[j] != GT) --j;
        int startFile = j + 1;
        std::string file = raw.substr(startFile, end - startFile + 1);

        // skip whitespace before operator (if any)
        while (j >= 0 && std::isspace(static_cast<unsigned char>(raw[j]))) --j;
        if (j < 0) break;

        char op = raw[j];
        if (op == LT) {
            // consume '<file' or '< file'
            --j; changed = true; inFile = file;
            raw.erase(j + 1); // remove from '<' to end
            continue;
        }
        if (op == GT) {
            int opEnd = j; // position of rightmost '>'
            --j;
            bool dbl = (j >= 0 && raw[j] == GT);
            int opStart = dbl ? j : opEnd;
            if (dbl) --j;
            changed = true; outFile = file; appendOut = dbl;
            raw.erase(opStart); // remove from first '>' to end
            continue;
        }
        // not a redirection tail
        break;
    }
    trimRight(raw);
}

std::istream& operator>>(std::istream& in, Stream& stream) {
    // Reset previous parsed list
    stream.clear();

    std::string line;
    if (!std::getline(in, line)) return in;

    if (line.empty()) {
        std::cerr << "\nError code 10 - Empty input" << std::endl;
        return in;
    }
    if (line.size() > MAX_SIZE) {
        std::cerr << "\nError code 11 - Input size exceeds maximum size" << std::endl;
        return in;
    }

    stream.split(line);

    for (auto& rawSeg : stream.pipeline) {
        std::string raw = rawSeg;
        std::string inFile, outFile; bool appendOut = false;
        parseRedirections(raw, inFile, outFile, appendOut);

        InputStream* node = new InputStream(raw);
        // attach redirections
        node->setInRedirect(inFile);
        node->setOutRedirect(outFile, appendOut);

        const std::string cmd = node->getCommand();

        // For echo/wc, if no explicit arg, provide it from input redirection or heredoc/multiline
        if (cmd == "echo" || cmd == "wc") {
            if (!node->hasExplicitArgument()) {
                if (!inFile.empty()) {
                    std::ifstream f(inFile);
                    if (!f.is_open()) {
                        std::cerr << "\nError code 5 - Could not open file: " << inFile << std::endl;
                    } else {
                        std::string line2, acc;
                        while (std::getline(f, line2)) { if (!acc.empty()) acc += '\n'; acc += line2; }
                        f.close();
                        node->setArgument("\"" + acc + "\"");
                    }
                } else if (node->getArgument().empty()) {
                    // heredoc-style: accept lines until blank line or a line that is exactly "EOF"
                    std::string acc;
                    std::string extra;
                    while (std::getline(in, extra)) {
                        if (extra == "EOF" || extra.empty()) {
                            node->setArgument("\"" + acc + "\"");
                            break;
                        }
                        if (!acc.empty()) acc += '\n';
                        acc += extra;
                    }
                }
            }
        }

        // For echo/wc with explicit unquoted .txt argument, read file content
        if ((cmd == "echo" || cmd == "wc") && isTxtFileCandidate(node->getArgument())) {
            FileStream* fnode = new FileStream(node->getCommand(), node->getOption(), node->getArgument());
            // preserve redirections on new node
            fnode->setInRedirect(inFile);
            fnode->setOutRedirect(outFile, appendOut);
            delete node;
            node = fnode;
        }

        // For tr: if first argument is an unquoted .txt file, load its content into argument (quoted)
        if (cmd == "tr" && isTxtFileCandidate(node->getArgument())) {
            FileStream* fnode = new FileStream(node->getCommand(), node->getOption(), node->getArgument());
            // we need to preserve argument2 and argument3 from the original parse
            // Since FileStream only sets argument, we copy over arg2/arg3 after reading
            std::string arg2 = node->getArgument2();
            std::string arg3 = node->getArgument3();
            fnode->setInRedirect(inFile);
            fnode->setOutRedirect(outFile, appendOut);
            delete node;
            node = fnode;
            // Restore arg2/arg3
            node->setArgument2(arg2);
            node->setArgument3(arg3);
        }

        stream.insert(node);
    }

    return in;
}

// ---------------- InputStream (node) ----------------
InputStream::InputStream(const std::string& line) { parse(line); }

void InputStream::appendArgumentLine(const std::string& line) {
    if (!argument.empty()) argument += '\n';
    argument += line;
}

std::string InputStream::nextToken(const std::string& line, size_t& i) {
    while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i]))) ++i;
    if (i >= line.size()) return {};

    std::string tok;
    if (line[i] == '"') {
        tok += line[i++];
        while (i < line.size()) {
            tok += line[i];
            if (line[i] == '"') { ++i; break; }
            ++i;
        }
    } else {
        while (i < line.size() && !std::isspace(static_cast<unsigned char>(line[i]))) {
            tok += line[i++];
        }
    }
    while (i < line.size() && std::isspace(static_cast<unsigned char>(line[i]))) ++i;
    return tok;
}

void InputStream::parse(const std::string& line) {
    size_t i = 0;
    command = nextToken(line, i);
    if (command.empty()) return;

    std::string second = nextToken(line, i);
    if (!second.empty() && second[0] == '-') {
        option = second;
        std::string third = nextToken(line, i);
        if (!third.empty()) { argument = third; setHasExplicitArgument(true); }
        // parse possible extra args when an option is used (rare in current commands)
        std::string fourth = nextToken(line, i);
        if (!fourth.empty()) argument2 = fourth;
        std::string fifth = nextToken(line, i);
        if (!fifth.empty()) argument3 = fifth;
    } else {
        if (!second.empty()) { argument = second; setHasExplicitArgument(true); }
        // parse possible extra args (for commands like tr)
        std::string third = nextToken(line, i);
        if (!third.empty()) argument2 = third;
        std::string fourth = nextToken(line, i);
        if (!fourth.empty()) argument3 = fourth;
    }
}

// ---------------- FileStream ----------------
FileStream::FileStream(const std::string& command,
                       const std::string& option,
                       const std::string& filePath) {
    setCommand(command);
    setOption(option);
    readFromFile(filePath);
}

void FileStream::readFromFile(const std::string& filePath) {
    try {
        std::ifstream f(filePath);
        if (!f.is_open()) {
            throw std::runtime_error("\nError code 5 - Could not open file: " + filePath);
        }
        std::string line;
        std::string acc;
        while (std::getline(f, line)) {
            if (!acc.empty()) acc += '\n';
            acc += line;
        }
        f.close();
        setArgument("\"" + acc + "\"");
    }
    catch (const std::exception& e) {
        std::cerr << "\nError code 6 - Error reading from file " << e.what() << std::endl;
    }
}