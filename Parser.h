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
	std::string parseArgument2();
	std::string parseArgument3();

	// Redirection accessors
	std::string parseInRedirect();
	std::string parseOutRedirect();
	bool parseAppendOut();
	bool parseHasExplicitArgument();


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

inline std::string Parser::parseArgument2() {
	return stream->getFirst()->getArgument2();
}

inline std::string Parser::parseArgument3() {
	return stream->getFirst()->getArgument3();
}

inline std::string Parser::parseInRedirect() {
	return stream->getFirst()->getInRedirect();
}

inline std::string Parser::parseOutRedirect() {
	return stream->getFirst()->getOutRedirect();
}

inline bool Parser::parseAppendOut() {
	return stream->getFirst()->isAppendOut();
}

inline bool Parser::parseHasExplicitArgument() {
	return stream->getFirst()->hasExplicitArgument();
}


#endif // !_PARSER_H_
