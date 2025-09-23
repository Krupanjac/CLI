#include <string>
#include <iostream>
#include <fstream>

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

    for (auto& raw : stream.pipeline) {
        InputStream* node = new InputStream(raw);

        // Multiline acquisition for echo / wc with missing argument
        if ((node->getCommand() == "echo" || node->getCommand() == "wc") && node->getArgument().empty()) {
            std::string acc;
            std::string extra;
            while (std::getline(in, extra)) {
                if (extra.empty()) { // terminate multiline
                    node->setArgument("\"" + acc + "\"");
                    break;
                }
                if (!acc.empty()) acc += '\n';
                acc += extra;
            }
        }
        else if ((node->getCommand() == "echo" || node->getCommand() == "wc") && isTxtFileCandidate(node->getArgument())) {
            // Replace with FileStream that reads file content
            FileStream* fnode = new FileStream(node->getCommand(), node->getOption(), node->getArgument());
            delete node;
            node = fnode;
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
        argument = nextToken(line, i);
    } else {
        argument = second;
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