#ifndef COMMAND_H_
#define COMMAND_H_

#include "Types.h"

#include <string>



//Abstract class for Control elements
class Command {
public:
	Command();

	//Constructor call for Control
	Command(std::string&, std::string&, std::string&, PSIGN&);

	//Destructor call for Control
	//virtual ~Command();

	//Method for calling exec on command
	virtual void execute() = 0;



protected:

	std::string command;
	std::string argument;
	std::string option;

	PSIGN prompt;


};

//Prompt command class for user to define beggining of a new command
class PromptCommand : public Command {
public:

	//Default constructor for Prompt with arguments 
	PromptCommand(PSIGN& p);

	//Destructor for Prompt
	~PromptCommand() = default;

	//Method for calling exec on Prompt
	void execute() override;


};

//Echo command class for user to define beggining of a new command
class EchoCommand : public Command {
public:

	//Default constructor for Echo
	EchoCommand(std::string);


	void execute() override;

private:
	std::string echoArgument;
};

class TimeCommand : public Command {
public:

	//Default constructor for Time
	TimeCommand();

	//Exec method for Time
	void execute() override;
};

class DateCommand : public Command {
public:

	//Default constructor for Date
	DateCommand();

	//Exec method for Date
	void execute() override;
};

class ClearCommand : public Command {
public:

	//Default constructor for Clear
	ClearCommand();

	//Exec method for Clear
	void execute() override;
};

class ExitCommand : public Command {
public:

	//Default constructor for Exit
	ExitCommand();

	//Exec method for Exit
	void execute() override;
};

class TouchCommand : public Command {
public:

	//Default constructor for Touch
	TouchCommand(std::string);

	//Exec method for Touch
	void execute() override;

private:
	std::string file;


};

class WcCommand : public Command {
public:

	//Default constructor for Wc
	WcCommand(std::string, std::string);

	//Exec method for Wc
	void execute() override;

	//Method for counting words
	void countWords();

	//Method for counting characters
	void countChars();

private:
	std::string argument;
	char option;
};

class HelpCommand : public Command {
public:

	//Default constructor for Help
	HelpCommand();

	//Exec method for Help
	void execute() override;
};

class TruncateCommand : public Command {
public:
	//Default constructor for Truncate
	TruncateCommand(std::string);

	//Exec method for Truncate
	void execute() override;
private:
	std::string file;
};


class RmCommand : public Command {
public:
	//Default constructor for rm
	RmCommand(std::string);

	//Exec method for rm
	void execute() override;
private:
	std::string file;
};

// tr command: replace all occurrences of what with with in input text
class TrCommand : public Command {
public:
	TrCommand(std::string inputText, std::string what, std::string with);
	void execute() override;
private:
	std::string input;
	std::string whatStr;
	std::string withStr; // may be empty -> removal
};


#endif // !COMMAND_H_
