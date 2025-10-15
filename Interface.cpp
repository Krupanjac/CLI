#include "Interface.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include "Stream.h"
#include "PipelineExecutor.h"

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
	PipelineExecutor executor;
	while (true) {
		command = new PromptCommand(p);
		command->execute();
		delete command;
		command = nullptr;

		parser = new Parser();
		executor.run(*parser, p);

		delete parser;
		parser = nullptr;
	}
}
