#ifndef COMMANDFACTORY_H_
#define COMMANDFACTORY_H_
#include "Command.h"
#include "Types.h"

enum ErrorCode {
	SUCCESS,
	INVALID_ARGUMENT,
	INVALID_OPTION,
	SYNTAX_ERROR,
	UNKNOWN_COMMAND
};

class CommandFactory {
public:
	CommandFactory();
	~CommandFactory();

	//Method for creating a command based on Fabric. Static.
	Command* createCommand(const std::string&,const std::string&,const std::string&, PSIGN &p);


	//Error code handler method
	void handleCommand(ErrorCode, const std::string&, const std::string&, const std::string&);

	//Print error context method
	void printErrorContext(const std::string&, const std::string&, const std::string&, const std::string&, ErrorCode);

private:
	//Validators for each command
	ErrorCode validatePromptCommand(const std::string& command, const std::string& option, const std::string& argument, PSIGN& prompt);
	ErrorCode validateEchoCommand(const std::string& command, const std::string& option, const std::string& argument);
	ErrorCode validateSimpleCommand(const std::string& command, const std::string& option, const std::string& argument);
	ErrorCode validateUnknownCommand(const std::string& command, const std::string& option, const std::string& argument);
	ErrorCode validateTouchCommand(const std::string& command, const std::string& option, const std::string& argument);
	ErrorCode validateWcCommand(const std::string& command, const std::string& option, const std::string& argument);

};

inline CommandFactory::CommandFactory() {

}

inline CommandFactory::~CommandFactory() {



}




#endif // !COMMANDFACTORY_H_



