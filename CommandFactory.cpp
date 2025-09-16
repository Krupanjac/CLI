#include "CommandFactory.h"
#include <iostream>
#include <string>







void CommandFactory::printErrorContext(const std::string& command, const std::string& option, const std::string& argument, const std::string& errorMessage, ErrorCode code) {
	char uChar = '~';
	int size = 0;
	if (command != "wc")
	std::cerr << command << " " << option << " " << argument << std::endl;
	else std::cerr << command << " " << option << " " << "Stream->File()" << std::endl;

	std::string underline;

	if (code == INVALID_ARGUMENT) {
		size = command.size() + option.size() + 1;

		underline.append(size, ' ');
		underline.append(argument.size(), uChar);

	}
	else if (code == INVALID_OPTION) {
		size = command.size() + 1;
		underline.append(size, ' ');
		underline.append(option.size(), uChar);
	}
	else if (code == UNKNOWN_COMMAND) {
		size = command.size();
		underline.append(size, uChar);
	}

	else if (code == SYNTAX_ERROR) {
		size = command.size() + option.size() + 1;
		underline.append(size, ' ');
		underline.append(argument.size(), uChar);
	}

std::cerr << underline << std::endl;

underline = "";
underline.append(size, ' ');

std::cerr << underline << "|" << std::endl;
std::cerr << underline << "|" << std::endl;
std::cerr << underline << "|" << std::endl;


std::cerr << underline + errorMessage << std::endl;
}





void CommandFactory::handleCommand(ErrorCode code, const std::string& command, const std::string& option, const std::string& argument) {
	std::string errorMessage;


	std::cout << std::endl;
	switch (code) {
	case INVALID_ARGUMENT:
		errorMessage = "Error code 1 - Invalid argument";
		break;
	case INVALID_OPTION:
		errorMessage = "Error code 2 - Invalid option";
		break;
	case SYNTAX_ERROR:
		errorMessage = "Error code 3 - Syntax error";
		break;
	case UNKNOWN_COMMAND:
		errorMessage = "Error code 4 - Unknown command";
		break;
	default:
		errorMessage = "Error code 0 -Segmentation fault";
		break;
	}
	printErrorContext(command, option, argument, errorMessage, code);
}

ErrorCode CommandFactory::validatePromptCommand(const std::string& command, const std::string& option, const std::string& argument, PSIGN& prompt) {
	if (!option.empty() || argument.empty()) {
		handleCommand(INVALID_ARGUMENT, command, option, argument);
		return INVALID_ARGUMENT;
	}
	prompt = argument[0];
	return SUCCESS;
}

ErrorCode CommandFactory::validateEchoCommand(const std::string& command, const std::string& option, const std::string& argument) {
	if (!option.empty()) {
		handleCommand(INVALID_OPTION, command, option, argument);
		return INVALID_OPTION;
	}


	else if (argument.empty() ||
		((argument.back() == '"') && (argument.front() != '"')) ||
		((argument.back() != '"') && (argument.front() == '"')) ||
		((argument.back() != '"') && (argument.front() != '"')))
	{
		handleCommand(SYNTAX_ERROR, command, option, argument);
		return SYNTAX_ERROR;
	}

	return SUCCESS;
}

ErrorCode CommandFactory::validateSimpleCommand(const std::string& command, const std::string& option, const std::string& argument) {
	if (!option.empty() || !argument.empty()) {
		handleCommand(INVALID_ARGUMENT, command, option, argument);
		return INVALID_ARGUMENT;
	}
	return SUCCESS;
}

ErrorCode CommandFactory::validateUnknownCommand(const std::string& command, const std::string& option, const std::string& argument) {
	if (!option.empty() || !argument.empty()) {
		handleCommand(UNKNOWN_COMMAND, command, option, argument);
		return UNKNOWN_COMMAND;
	}
	return SUCCESS;

}

ErrorCode CommandFactory::validateTouchCommand(const std::string& command, const std::string& option, const std::string& argument) {
	if (!option.empty()) {
		handleCommand(INVALID_OPTION, command, option, argument);
		return INVALID_OPTION;
	}
	else if (argument.empty()) {
		handleCommand(INVALID_ARGUMENT, command, option, argument);
		return INVALID_ARGUMENT;
	}

	else if (argument.front() == '"' || argument.back() == '"') {
		handleCommand(SYNTAX_ERROR, command, option, argument);
		return SYNTAX_ERROR;
	}

	return SUCCESS;
}

ErrorCode CommandFactory::validateWcCommand(const std::string& command, const std::string& option, const std::string& argument) {


	if (option.length() > 1 && option[0] != '-' ||
		(option.length() > 1 && option[1] != 'c' && option[1] != 'w')) {
		handleCommand(INVALID_OPTION, command, option, argument);
		return INVALID_OPTION;
		}
	
	else if (argument.empty() ||
		(argument.length() <2) ||
		(argument.front() != '"' && argument.back() == '"') ||
		(argument.front() == '"' && argument.back() != '"') ||
		(argument.front() != '"' && argument.back() != '"'))
	{
		handleCommand(INVALID_ARGUMENT, command, option, argument);
		return INVALID_ARGUMENT;
	}


	return SUCCESS;
}


Command* CommandFactory::createCommand(const std::string& command, const std::string& option, const std::string& argument, PSIGN& prompt) {
    if (command == "prompt") {
        if (validatePromptCommand(command, option, argument, prompt) == SUCCESS) {
            return nullptr;
        }
    }
    else if (command == "echo") {
        if (validateEchoCommand(command, option, argument) == SUCCESS) {
            return new EchoCommand(argument);
        }
    }
    else if (command == "time") {
        if (validateSimpleCommand(command, option, argument) == SUCCESS) {
            return new TimeCommand();
        }
    }
    else if (command == "date") {
        if (validateSimpleCommand(command, option, argument) == SUCCESS) {
            return new DateCommand();
        }
    }
    else if (command == "clear") {
        if (validateSimpleCommand(command, option, argument) == SUCCESS) {
            return new ClearCommand();
        }
    }
    else if (command == "touch") {
        if (validateTouchCommand(command, option, argument) == SUCCESS) {
            return new TouchCommand(argument);
        }
    }
    else if (command == "wc") {
        if (validateWcCommand(command, option, argument) == SUCCESS) {
            return new WcCommand(argument, option);
        }
    }
    else if (command == "exit") {
        if (validateSimpleCommand(command, option, argument) == SUCCESS) {
            return new ExitCommand();
        }
    }

	else if (command == "help") {
		if (validateSimpleCommand(command, option, argument) == SUCCESS) {
			return new HelpCommand();
		}
	}

	else if (command == "fun") {
		if (validateSimpleCommand(command, option, argument) == SUCCESS) {
			return new FunCommand();
		}
	}

    else {
		handleCommand(UNKNOWN_COMMAND, command, option, argument);
    }
    return nullptr;
}

