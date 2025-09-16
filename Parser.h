#ifndef _PARSER_H_
#define _PARSER_H_

#include "Stream.h"

//Handle stream object
class Parser {

public:
	Parser();
	~Parser();

	//Process a single stream
	bool processOne();

	//Process ALL streams (if command | command | command exists)
	bool processAll();

	//Sends command to CLI
	std::string parseCommand();
	
	//Sends option to CLI
	std::string parseOption();

	//Sends argument to CLI
	std::string parseArgument();


private:
	Stream* stream;



};

inline std::string Parser::parseCommand() {
	return stream->getFirst()->getCommand();
}

inline std::string Parser::parseOption() {
	return stream->getFirst()->getOption();
}

inline std::string Parser::parseArgument() {
	return stream->getFirst()->getArgument();
}



#endif // !_PARSER_H_
