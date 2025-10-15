#ifndef _PARSER_H_
#define _PARSER_H_

#include "Stream.h"

//Handle stream object
class Parser {
public:
	Parser();
	~Parser() = default;

	//Process a single stream
	bool processOne();

	//Process ALL streams (if command | command | command exists)
	bool processAll();

	
	InputStream* getCurrent();
	InputStream* getNext();

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
	Stream stream;
};
inline InputStream* Parser::getCurrent() { return stream.current(); }
inline InputStream* Parser::getNext() { return stream.next(); }
inline std::string Parser::parseCommand() { return stream.current()->getCommand(); }
inline std::string Parser::parseOption() { return stream.current()->getOption(); }
inline std::string Parser::parseArgument() { return stream.current()->getArgument(); }
inline std::string Parser::parseArgument2() { return stream.current()->getArgument2(); }
inline std::string Parser::parseArgument3() { return stream.current()->getArgument3(); }

inline std::string Parser::parseInRedirect() { return stream.current()->getInRedirect(); }
inline std::string Parser::parseOutRedirect() { return stream.current()->getOutRedirect(); }
inline bool Parser::parseAppendOut() { return stream.current()->isAppendOut(); }
inline bool Parser::parseHasExplicitArgument() { return stream.current()->hasExplicitArgument(); }

#endif // !_PARSER_H_
