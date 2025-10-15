#include "PipelineExecutor.h"
#include <fstream>
#include <iostream>
#include <sstream>

void PipelineExecutor::run(Parser& parser, PSIGN& p) {
    do {
        std::string comm = parser.parseCommand();
        std::string arg = parser.parseArgument();
        std::string opt = parser.parseOption();
        std::string arg2 = parser.parseArgument2();
        std::string arg3 = parser.parseArgument3();

        std::string inRedir = parser.parseInRedirect();
        std::string outRedir = parser.parseOutRedirect();
        bool appendOut = parser.parseAppendOut();
        bool hasExplicitArg = parser.parseHasExplicitArgument();

        if (!inRedir.empty()) {
            bool definesInput = (comm == "echo" || comm == "wc" || comm == "head" || comm == "batch");
            if (!definesInput || hasExplicitArg) {
                factory.handleCommand(SYNTAX_ERROR, comm, opt, arg);
                continue;
            }
        }

        Command* command = factory.createCommand(comm, opt, arg, arg2, arg3, p);
        if (command) {
            std::streambuf* oldBuf = nullptr;
            std::ofstream outFile;
            bool wroteToFile = false;

            InputStream* currentNode = parser.getCurrent();
            InputStream* nextNode = parser.getNext();
            bool hasNextInPipe = (nextNode != nullptr);
            std::ostringstream capture;
            bool capturing = false;

            if (hasNextInPipe && outRedir.empty()) {
                oldBuf = std::cout.rdbuf(capture.rdbuf());
                capturing = true;
            } else if (!outRedir.empty()) {
                std::ios_base::openmode mode = std::ios::out;
                mode |= appendOut ? std::ios::app : std::ios::trunc;
                outFile.open(outRedir, mode);
                if (!outFile.is_open()) {
                    std::cerr << "Error code 8 - Could not open output file '" << outRedir << "'" << std::endl;
                } else {
                    oldBuf = std::cout.rdbuf(outFile.rdbuf());
                    wroteToFile = true;
                }
            }

            command->execute();

            if (oldBuf) {
                std::cout.flush();
                std::cout.rdbuf(oldBuf);
                if (outFile.is_open()) outFile.close();
            }

            if (wroteToFile) {
                if (outRedir.size() >= 4 && outRedir.substr(outRedir.size() - 4) == ".txt") {
                    std::ifstream inFile(outRedir);
                    if (inFile.is_open()) {
                        std::string line;
                        bool first = true;
                        while (std::getline(inFile, line)) {
                            if (!first) std::cout << '\n';
                            std::cout << line;
                            first = false;
                        }
                        std::cout << std::endl;
                        inFile.close();
                    }
                }
            }

            if (hasNextInPipe && capturing) {
                std::string produced = capture.str();
                if (!produced.empty() && produced.back() == '\n') {
                    produced.pop_back();
                }
                std::string quoted = std::string("\"") + produced + "\"";

                std::string nextCmd = nextNode->getCommand();
                if (nextCmd == "tr") {
                    std::string a = nextNode->getArgument();
                    std::string b = nextNode->getArgument2();
                    std::string c = nextNode->getArgument3();
                    nextNode->setArgument(quoted);
                    if (c.empty()) {
                        nextNode->setArgument2(a);
                        nextNode->setArgument3(b);
                    }
                } else if (nextCmd == "echo" || nextCmd == "wc" || nextCmd == "head" || nextCmd == "batch") {
                    nextNode->setArgument(quoted);
                }
            }

            delete command;
            command = nullptr;
        }
    } while (parser.processAll());
}
