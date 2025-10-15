#include <iostream>
#include <cmath>
#include <cstring>
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "Command.h"
#include "Stream.h"
#include "CommandFactory.h"

//Prompt class definition

void PromptCommand::execute() {
	std::cout << prompt << " ";
}


//Echo class definition


void EchoCommand::execute() {

	if (echoArgument.front() != '"')
		std::cout << echoArgument << std::endl;

	else std::cout << echoArgument.substr(1, echoArgument.length() - 2) << std::endl;

}

//Time class definition

void TimeCommand::execute() {
	std::time_t now = std::time(nullptr);
	std::tm localTm{};
	localtime_s(&localTm, &now);
	std::cout << "Time: " << std::put_time(&localTm, "%H:%M:%S") << std::endl;
}

//Date class definition

void DateCommand::execute() {
	time_t now = time(0);
	tm localTm{};
	localtime_s(&localTm, &now);
	std::cout << "Date: " << std::put_time(&localTm, "%d-%m-%Y") << std::endl;
}

//Touch class definition

void TouchCommand::execute() {
	try {
		// Check if the file exists
		std::ifstream infile(file);
		if (infile) {  // File exists
			throw std::runtime_error("7 - File '" + file + "' already exists.");
		}

		// Try to create the file
		std::ofstream outfile(file);
		if (!outfile) {
			throw std::runtime_error("8 - Could not create file '" + file + "'");
		}
	}
	catch (const std::runtime_error& e) {

		std::cerr << "Error code " << e.what() << std::endl;
		return;
	}
}


 //Wc class definition

void WcCommand::execute() {
	if (option == 'w') countWords();
	else countChars();
}

void WcCommand::countWords() {
	int count = 0;
	bool inWord = false;
	std::string data = argument;
	if (!data.empty() && data.front() == '"' && data.back() == '"') {
		data = data.substr(1, data.size()-2);
	}
	for (char c : data) {
		if (std::isspace(static_cast<unsigned char>(c))) {
			if (inWord) { ++count; inWord = false; }
		} else {
			inWord = true;
		}
	}
	if (inWord) ++count;
	std::cout << count << std::endl;
}

void WcCommand::countChars() {
	std::string data = argument;
	if (!data.empty() && data.front() == '"' && data.back() == '"') data = data.substr(1, data.length() - 2);
	std::cout << data.length() << std::endl;
}



///Help class definition

void HelpCommand::execute() {
	std::cout << std::endl;
	std::cout << "time - prints the current time" << std::endl;
	std::cout << "date - prints the current date" << std::endl;
	std::cout << "clear - clears the screen" << std::endl;
	std::cout << "exit - exits the program" << std::endl;
	std::cout << "echo [argument] - prints the argument" << std::endl;
	std::cout << "touch [argument] - creates a new file" << std::endl;
	std::cout << "prompt [argument] - changes the prompt" << std::endl;
	std::cout << "wc [-opt] [argument] - prints the word or character count" << std::endl;
	std::cout << "truncate [argument] - truncates the file to zero length" << std::endl;
	std::cout << "rm [argument] - deletes the file" << std::endl;
	std::cout << "tr [argument] [what] [with] - replace substrings in input" << std::endl;
	std::cout << "head -n<number> [argument] - prints the first N lines" << std::endl;
	std::cout << "batch [argument] - executes each line as a command" << std::endl;

	std::cout << std::endl << "[argument] - a single word, a quoted string or filename" << std::endl;
	std::cout << "[-opt] - a single character option." << std::endl;

}

//Truncate class definition


void TruncateCommand::execute() {
	std::ofstream ofs;
	ofs.open(file, std::ofstream::out | std::ofstream::trunc);
	ofs.close();
}


///RmCommand class definition

void RmCommand::execute() {
	if (std::remove(file.c_str()) != 0) {
		std::cerr << "Error deleting file: " << file << std::endl;
	}
	else {
		std::cout<< file << " deleted successfully!" << std::endl;
	}
}

// Utility function to remove surrounding quotes from a string if present
static std::string unquote(const std::string& s) {
	if (s.size() >= 2 && s.front() == '"' && s.back() == '"') return s.substr(1, s.size() - 2);
	return s;
}
// TrCommand implementation

void TrCommand::execute() {
	std::string text = unquote(input);
	std::string what = unquote(whatStr);
	std::string with = unquote(withStr);

	if (what.empty()) {
		std::cout << text << std::endl;
		return;
	}

	// Replace all occurrences
	int pos = 0;
	while ((pos = static_cast<int>(text.find(what, pos))) != std::string::npos) {
		text.replace(pos,what.size(), with);
		pos += static_cast<int>(with.size());
	}
	std::cout << text << std::endl;
}

// HeadCommand implementation

void HeadCommand::execute() {
	std::string text = unquote(input);
	if (n <= 0) {
		std::cout << std::endl;
		return;
	}
	int printed = 0;
	std::string line;
	for (int i = 0; i <= text.size(); ++i) {
		if (i == text.size() || text[i] == '\n') {
			std::cout << line;
			++printed;
			if (printed >= n) {
				std::cout << std::endl; // end with newline
				return;
			}
			std::cout << '\n';
			line.clear();
		} else {
			line += text[i];
		}
	}
	// If fewer than n lines exist, still end with newline
	if (printed < n) std::cout << std::endl;
}

// BatchCommand implementation

void BatchCommand::execute() {
	std::string all = unquote(input);
	std::istringstream iss(all);
	std::string rawLine;
	PSIGN p = '$';
	CommandFactory factory;
	while (std::getline(iss, rawLine)) {
		if (rawLine.empty()) continue;
		// Parse this line into a local Stream using operator>>
		Stream s;
		s.clear();
		std::istringstream one(rawLine);
		one >> s;

		// Execute all nodes parsed from the line (to support pipelines or multiple segments if any)
		while (!s.empty()) {
			InputStream* node = s.current();
			std::string comm = node->getCommand();
			std::string arg = node->getArgument();
			std::string opt = node->getOption();
			std::string inRedir = node->getInRedirect();
			std::string outRedir = node->getOutRedirect();
			bool appendOut = node->isAppendOut();
			bool hasExplicitArg = node->hasExplicitArgument();

			if (!inRedir.empty()) {
				bool definesInput = (comm == "echo" || comm == "wc" || comm == "head" || comm == "batch");
				if (!definesInput || hasExplicitArg) {
					factory.handleCommand(SYNTAX_ERROR, comm, opt, arg);
					s.advance();
					continue;
				}
			}

			Command* cmd = nullptr;
			if (comm == "tr") {
				cmd = factory.createCommand(comm, opt, arg, node->getArgument2(), node->getArgument3(), p);
			} else {
				cmd = factory.createCommand(comm, opt, arg, p);
			}
			if (cmd) {
				std::streambuf* oldBuf = nullptr;
				std::ofstream outFile;
				bool wroteToFile = false;

				InputStream* nextNode = s.next();
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
					if (outFile.is_open()) { oldBuf = std::cout.rdbuf(outFile.rdbuf()); wroteToFile = true; }
				}

				cmd->execute();

				if (oldBuf) {
					std::cout.flush();
					std::cout.rdbuf(oldBuf);
					if (outFile.is_open()) outFile.close();
				}

				if (wroteToFile && appendOut) {
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

				// pipeline forwarding into next node
				if (hasNextInPipe && capturing) {
					std::string produced = capture.str();
					if (!produced.empty() && produced.back() == '\n') produced.pop_back();
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

				delete cmd;
			}

			s.advance();
		}
	}
}