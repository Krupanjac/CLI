#ifndef COMMAND_H_
#define COMMAND_H_

#include "Types.h"

#include <string>



//Abstract class for Control elements
class Command {
public:
	// Default constructor
	Command();

	// Parameterized constructor
	Command(std::string& comm, std::string& opt, std::string& arg, PSIGN& p);

	virtual void execute() = 0;

protected:

	std::string command;
	std::string argument;
	std::string option;
	PSIGN prompt;
};
inline Command::Command(std::string& comm, std::string& opt, std::string& arg, PSIGN& p)
	: command(comm), argument(arg), option(opt), prompt(p) {
}
inline Command::Command() : command(""), argument(""), option(""), prompt('$') {
}

//Prompt command class for user to define beggining of a new command
class PromptCommand : public Command {
public:


	PromptCommand(PSIGN& p);
	~PromptCommand() = default;

	void execute() override;


};

inline PromptCommand::PromptCommand(PSIGN& p) {
	prompt = p;
}


//Echo command class for user to define beggining of a new command
class EchoCommand : public Command {
public:


	EchoCommand(std::string);


	void execute() override;

private:
	std::string echoArgument;
};

inline EchoCommand::EchoCommand(std::string arg) : echoArgument(arg) {
}

class TimeCommand : public Command {
public:


	TimeCommand() = default;

	void execute() override;
};

class DateCommand : public Command {
public:

	DateCommand() = default;

	void execute() override;
};

class ClearCommand : public Command {
public:

	//Default constructor for Clear
	ClearCommand() = default;

	void execute() override;
};

inline void ClearCommand::execute() {
	system("cls");
}

class ExitCommand : public Command {
public:

	//Default constructor for Exit
	ExitCommand() = default;

	//Exec method for Exit
	void execute() override;
};

inline void ExitCommand::execute() {
	exit(0);
}

class TouchCommand : public Command {
public:

	//Default constructor for Touch
	TouchCommand(std::string);

	//Exec method for Touch
	void execute() override;

private:
	std::string file;


};

inline TouchCommand::TouchCommand(std::string arg) : file(arg) {
}

class WcCommand : public Command {
public:

	WcCommand(std::string, std::string);

	void execute() override;

	//Method for counting words
	void countWords();

	//Method for counting characters
	void countChars();

private:
	std::string argument;
	char option;
};

inline WcCommand::WcCommand(std::string arg, std::string opt) : argument(arg), option(opt.empty() ? ' ' : opt[1]) {
}

class HelpCommand : public Command {
public:

	HelpCommand() = default;
	void execute() override;
};

class TruncateCommand : public Command {
public:

	TruncateCommand(std::string);
	void execute() override;

private:
	std::string file;
};

inline TruncateCommand::TruncateCommand(std::string filename) : file(filename) {
}


class RmCommand : public Command {
public:

	RmCommand(std::string);
	void execute() override;
private:
	std::string file;
};

inline RmCommand::RmCommand(std::string filename) : file(filename) {
}

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

inline TrCommand::TrCommand(std::string inputText, std::string what, std::string with)
	: input(std::move(inputText)), whatStr(std::move(what)), withStr(std::move(with)) {
}

// head command: print first n lines of input text
class HeadCommand : public Command {
public:
	HeadCommand(std::string inputText, int nLines);
	void execute() override;
private:
	std::string input;
	int n;
};
inline HeadCommand::HeadCommand(std::string inputText, int nLines)
	: input(std::move(inputText)), n(nLines) {
}


// batch command: execute each line as a command line
class BatchCommand : public Command {
public:
	explicit BatchCommand(std::string inputText);
	void execute() override;
private:
	std::string input;
};

inline BatchCommand::BatchCommand(std::string inputText)
	: input(std::move(inputText)) {
}

#endif // !COMMAND_H_
