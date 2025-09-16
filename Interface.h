#ifndef INTEFACE_H_
#define INTERFACE_H_
#include "Parser.h"
#include "Command.h"
#include "Types.h"
#include "CommandFactory.h"


class Interface {

public:


	Interface();

	~Interface();

	//Runs the interface
	void run();



private:
	Parser* parser;
	Command* command;
 
};





#endif // !INTEFACE_H_

