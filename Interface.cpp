#include "Interface.h"
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

			command = factory.createCommand(comm, opt, arg, p);
			if (command) {
				command->execute();
				delete command;
				command = nullptr;
			}
		} while (parser->processAll());

		delete parser;
		parser = nullptr;
	}
}
