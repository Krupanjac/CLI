#include <iostream>
#include <cmath>
#include <cstring>
#include <fstream>
#include "Command.h"

//Command class definition
Command::Command(std::string& comm, std::string& opt, std::string& arg, PSIGN& p) : command(comm), option(opt), argument(arg), prompt(p) {

}

Command::Command() : command(""), option(""), argument(""), prompt('$') {
}



//Prompt class definition

PromptCommand::PromptCommand(PSIGN& p) {
	prompt = p;
	// execute() removed from constructor (now called explicitly)
}

void PromptCommand::execute() {
	std::cout << prompt << " ";
}


//Echo class definition

EchoCommand::EchoCommand(std::string arg) : echoArgument(arg) {
	// execute() removed from constructor (now called explicitly)
}


void EchoCommand::execute() {

	if (echoArgument.front() != '"')
		std::cout << echoArgument << std::endl;

	else std::cout << echoArgument.substr(1, echoArgument.length() - 2) << std::endl;

}

//Time class definition

TimeCommand::TimeCommand() {
	// execute() removed from constructor
}

void TimeCommand::execute() {
	std::cout << "Time: " << __TIME__ << std::endl;
}

//Date class definition

DateCommand::DateCommand() {
	// execute() removed from constructor
}

void DateCommand::execute() {
	std::cout << "Date: " << __DATE__ << std::endl;
}

//Clear class definition

ClearCommand::ClearCommand() {
	// execute() removed from constructor
}

void ClearCommand::execute() {
	system("cls");
}

//Exit class definition

ExitCommand::ExitCommand() {
	// execute() removed from constructor
}

void ExitCommand::execute() {
	exit(0);
}

//Touch class definition

TouchCommand::TouchCommand(std::string arg) : file(arg) {
	// execute() removed from constructor
}

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

WcCommand::WcCommand(std::string arg, std::string opt) : argument(arg), option(opt[1]) {
	// execute() removed from constructor
}

void WcCommand::execute() {
	\
		if (option == 'w') countWords();

		else countChars();

}

void WcCommand::countWords() {
	int count = 1;

	for (int i = 0; i < argument.length(); i++) {
		if (argument[i] == ' ') {
			count++;
		}
	}

	std::cout << "Word count: " << count << std::endl;
}

void WcCommand::countChars() {
	if (argument[0] != '"')
		std::cout << "Character count: " << argument.length() << std::endl;
	else std::cout << "Character count: " << argument.length() - 2 << std::endl;
}


//Help class definition

HelpCommand::HelpCommand() {
	// execute() removed from constructor
}

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

	//argument explanation
	std::cout << std::endl << "[argument] - a single word, a quoted string or filename" << std::endl;

	//option explanation
	std::cout << "[-opt] - a single character option." << std::endl;

}

/*Truncate class definition*/
TruncateCommand::TruncateCommand(std::string filename) :file(filename) {

}

void TruncateCommand::execute() {
	std::ofstream ofs;
	ofs.open(file, std::ofstream::out | std::ofstream::trunc);
	ofs.close();
}

