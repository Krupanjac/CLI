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
	while (true) {
	

		Command* command = new PromptCommand(p);
		delete command;
		command = nullptr;

		parser = new Parser();

		CommandFactory factory;

		do{
		std::string comm = parser->parseCommand();
		std::string arg = parser->parseArgument();	
		std::string opt = parser->parseOption();
		
		

		command = factory.createCommand(comm, opt, arg, p);
		delete command;
		command = nullptr;
		} while (parser->processAll());
		delete parser;
		parser = nullptr;
	}

}
