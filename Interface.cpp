#include "Interface.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "Stream.h"

//Constructor for Interface
Interface::Interface() : parser(nullptr), command(nullptr) {
	run();
}

Interface::~Interface() {
	delete parser;
	parser = nullptr;
}

//Runs the interface
void Interface::run() {
	PSIGN p = '$';
	CommandFactory factory; // reuse factory each loop
	while (true) {
		// Show prompt explicitly
		command = new PromptCommand(p);
		command->execute();
		delete command;
		command = nullptr;

		parser = new Parser();

		do {
			std::string comm = parser->parseCommand();
			std::string arg = parser->parseArgument();
			std::string opt = parser->parseOption();

			// Redirection info
			std::string inRedir = parser->parseInRedirect();
			std::string outRedir = parser->parseOutRedirect();
			bool appendOut = parser->parseAppendOut();
			bool hasExplicitArg = parser->parseHasExplicitArgument();

			// Input redirection rule: only for echo/wc/head/batch when they don't already have an explicit argument
			if (!inRedir.empty()) {
				bool definesInput = (comm == "echo" || comm == "wc" || comm == "head" || comm == "batch");
				if (!definesInput || hasExplicitArg) {
					factory.handleCommand(SYNTAX_ERROR, comm, opt, arg);
					continue; // skip execution
				}
			}

			command = factory.createCommand(comm, opt, arg, p);
			if (command) {
				std::streambuf* oldBuf = nullptr;
				std::ofstream outFile;
				bool wroteToFile = false;

				// Determine if there is a next node in the pipeline to feed
				InputStream* currentNode = Stream::instance()->getFirst();
				InputStream* nextNode = currentNode ? currentNode->getNext() : nullptr;
				bool hasNextInPipe = (nextNode != nullptr);
				std::ostringstream capture;
				bool capturing = false;

				if (hasNextInPipe && outRedir.empty()) {
					// Capture stdout of this command to feed into next
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

				// If we redirected output to a .txt file, also print its contents after writing
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

				// Pipeline bridging: feed captured output to the next node as its input argument
				if (hasNextInPipe && capturing) {
					std::string produced = capture.str();
					// strip a single trailing newline to avoid accidental extra empty line accumulation
					if (!produced.empty() && produced.back() == '\n') {
						produced.pop_back();
					}
					std::string quoted = std::string("\"") + produced + "\"";

					std::string nextCmd = nextNode->getCommand();
					if (nextCmd == "tr") {
						// Shift tokens so that captured text becomes the first (input) argument
						std::string a = nextNode->getArgument();   // what
						std::string b = nextNode->getArgument2(); // with
						std::string c = nextNode->getArgument3(); // (unused)
						nextNode->setArgument(quoted);
						nextNode->setArgument2(a);
						nextNode->setArgument3(b.empty() ? c : b);
					} else if (nextCmd == "echo" || nextCmd == "wc" || nextCmd == "head" || nextCmd == "batch") {
						nextNode->setArgument(quoted);
					}
				}

				delete command;
				command = nullptr;
			}
		} while (parser->processAll());

		delete parser;
		parser = nullptr;
	}
}
