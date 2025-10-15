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

	// Primary creation API: pass up to three args (unused ones can be empty)
	Command* createCommand(const std::string&, const std::string&, const std::string&, const std::string&, const std::string&, PSIGN &p);
	
	void handleCommand(ErrorCode, const std::string&, const std::string&, const std::string&);
	void printErrorContext(const std::string&, const std::string&, const std::string&, const std::string&, ErrorCode);

private:
	ErrorCode validatePromptCommand(const std::string& command, const std::string& option, const std::string& argument, PSIGN& prompt);
	ErrorCode validateEchoCommand(const std::string& command, const std::string& option, const std::string& argument);
	ErrorCode validateSimpleCommand(const std::string& command, const std::string& option, const std::string& argument);
	ErrorCode validateUnknownCommand(const std::string& command, const std::string& option, const std::string& argument);
	ErrorCode validateTouchCommand(const std::string& command, const std::string& option, const std::string& argument);
	ErrorCode validateWcCommand(const std::string& command, const std::string& option, const std::string& argument);
	ErrorCode validateTruncateCommand(const std::string& command, const std::string& option, const std::string& argument);
	ErrorCode validateRmCommand(const std::string& command, const std::string& option, const std::string& argument);
	ErrorCode validateTrCommand(const std::string& command, const std::string& option, const std::string& inputArg, const std::string& whatArg, const std::string& withArg);
	ErrorCode validateHeadCommand(const std::string& command, const std::string& option, const std::string& argument);
	ErrorCode validateBatchCommand(const std::string& command, const std::string& option, const std::string& argument);

	bool validateFileForOpen(const std::string&, bool, bool, bool);
	bool isProperQuoted(const std::string&);
	bool hasUnterminatedQuote(const std::string&);
};

inline CommandFactory::CommandFactory() {}
inline CommandFactory::~CommandFactory() {}

inline bool CommandFactory::isProperQuoted(const std::string& s) {
	return (s.size() >= 2 && s.front() == '"' && s.back() == '"');
}

inline bool CommandFactory::hasUnterminatedQuote(const std::string& s) {
	if (s.empty()) return false;
	bool starts = s.front() == '"';
	bool ends = s.back() == '"';
	return (starts ^ ends); // exactly one quote
}

#endif // !COMMANDFACTORY_H_

















































