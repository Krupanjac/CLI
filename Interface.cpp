#include "Interface.h"
#include <fstream>
#include <iostream>

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
				if (!outRedir.empty()) {
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
					outFile.close();
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

				delete command;
				command = nullptr;
			}
		} while (parser->processAll());

		delete parser;
		parser = nullptr;
	}
}
