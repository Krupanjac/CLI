#include "CommandFactory.h"
#include <iostream>
#include <string>
#include <fstream>
#include "Stream.h"






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

	// Support both: prompt $  and  prompt \"$\"
	if (argument.size() >= 2 && argument.front() == '"' && argument.back() == '"') {
		std::string inner = argument.substr(1, argument.size() - 2);
		if (inner.size() == 1) {
			prompt = inner[0];
			return SUCCESS;
		}
		// Quoted but not a single character -> invalid
		handleCommand(INVALID_ARGUMENT, command, option, argument);
		return INVALID_ARGUMENT;
	}

	// Unquoted single character
	if (argument.size() == 1) {
		prompt = argument[0];
		return SUCCESS;
	}

	// Anything else is invalid
	handleCommand(INVALID_ARGUMENT, command, option, argument);
	return INVALID_ARGUMENT;
}

ErrorCode CommandFactory::validateEchoCommand(const std::string& command, const std::string& option, const std::string& argument) {
	if (!option.empty()) {
		handleCommand(INVALID_OPTION, command, option, argument);
		return INVALID_OPTION;
	}

	if (argument.empty()) {
		handleCommand(INVALID_ARGUMENT, command, option, argument);
		return INVALID_ARGUMENT;
	}

	// Only accept a properly quoted string as the final validated argument.
	// If a .txt filename was provided, the parser replaces it with quoted file contents before validation.
	bool quoted = argument.size() >= 2 && argument.front() == '"' && argument.back() == '"';
	if (quoted) return SUCCESS;

	// Any other unquoted token (e.g., words like asdsaas, symbols like >, >>, <, <<) is invalid for echo
	handleCommand(INVALID_ARGUMENT, command, option, argument);
	return INVALID_ARGUMENT;
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
	if(!validateFileForOpen(argument,false,true,false)){
		handleCommand(INVALID_ARGUMENT, command, option, argument);
		return INVALID_ARGUMENT;
	}
	// Do not pre-fail when the file already exists; let TouchCommand produce the proper error code (7)
	return SUCCESS;
}

ErrorCode CommandFactory::validateWcCommand(const std::string& command, const std::string& option, const std::string& argument) {

	// option validation
	if ((option.length() > 1 && option[0] != '-') ||
		(option.length() > 1 && option[1] != 'c' && option[1] != 'w')) {
		handleCommand(INVALID_OPTION, command, option, argument);
		return INVALID_OPTION;
	}

	// argument validation: wc expects its final evaluated argument to be quoted text
	// If a .txt filename was provided or input was redirected, the parser replaces it with quoted text.
	if (argument.empty() || argument.front() != '"' || argument.back() != '"') {
		handleCommand(INVALID_ARGUMENT, command, option, argument);
		return INVALID_ARGUMENT;
	}
	return SUCCESS;
}


ErrorCode CommandFactory::validateTruncateCommand(const std::string& command, const std::string& option, const std::string& argument) {
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
	if (!validateFileForOpen(argument, true, true, true)) {
		handleCommand(INVALID_ARGUMENT, command, option, argument);
		return INVALID_ARGUMENT;
	}
	return SUCCESS;
}


ErrorCode CommandFactory::validateRmCommand(const std::string& command, const std::string& option, const std::string& argument) {
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
	if (!validateFileForOpen(argument, true, true, false)) {
		handleCommand(INVALID_ARGUMENT, command, option, argument);
		return INVALID_ARGUMENT;
	}
	return SUCCESS;
}

// tr: argument must be quoted input text (parser ensures file -> quoted),
// what must be quoted, with can be empty or quoted. No options allowed.
ErrorCode CommandFactory::validateTrCommand(const std::string& command, const std::string& option, const std::string& inputArg, const std::string& whatArg, const std::string& withArg) {
	if (!option.empty()) {
		handleCommand(INVALID_OPTION, command, option, inputArg);
		return INVALID_OPTION;
	}
	if (inputArg.empty()) {
		handleCommand(INVALID_ARGUMENT, command, option, inputArg);
		return INVALID_ARGUMENT;
	}
	if (!(inputArg.size() >= 2 && inputArg.front() == '"' && inputArg.back() == '"')) {
		// input must be quoted text by the time we validate
		handleCommand(INVALID_ARGUMENT, command, option, inputArg);
		return INVALID_ARGUMENT;
	}
	if (whatArg.empty() || !(whatArg.size() >= 2 && whatArg.front() == '"' && whatArg.back() == '"')) {
		handleCommand(INVALID_ARGUMENT, command, option, whatArg);
		return INVALID_ARGUMENT;
	}
	if (!withArg.empty() && !(withArg.size() >= 2 && withArg.front() == '"' && withArg.back() == '"')) {
		handleCommand(INVALID_ARGUMENT, command, option, withArg);
		return INVALID_ARGUMENT;
	}
	return SUCCESS;
}


// head: option must be -n<digits up to 5>, argument must be quoted text (or loaded from file)
ErrorCode CommandFactory::validateHeadCommand(const std::string& command, const std::string& option, const std::string& argument) {
	if (option.size() < 2 || option[0] != '-') {
		handleCommand(INVALID_OPTION, command, option, argument);
		return INVALID_OPTION;
	}
	if (option.size() < 3 || option[1] != 'n') {
		handleCommand(INVALID_OPTION, command, option, argument);
		return INVALID_OPTION;
	}
	std::string digits = option.substr(2);
	if (digits.empty() || digits.size() > 5) {
		handleCommand(INVALID_OPTION, command, option, argument);
		return INVALID_OPTION;
	}
	for (char c : digits) {
		if (!std::isdigit(static_cast<unsigned char>(c))) {
			handleCommand(INVALID_OPTION, command, option, argument);
			return INVALID_OPTION;
		}
	}
	// argument must be quoted text
	if (argument.empty() || argument.front() != '"' || argument.back() != '"') {
		handleCommand(INVALID_ARGUMENT, command, option, argument);
		return INVALID_ARGUMENT;
	}
	return SUCCESS;
}


bool CommandFactory::validateFileForOpen(const std::string& path, bool requireExist, bool requireTxt, bool forWrite){
	if(path.empty()) return false;
	if(requireTxt){
		if(path.size() < 4) return false;
		if(path.substr(path.size()-4) != ".txt") return false;
	}
	if(requireExist){
		std::ifstream in(path);
		if(!in.good()) return false;
		if(!in.is_open()) return false;
	}
	if(forWrite){
		std::ofstream out(path, std::ios::app);
		if(!out.good()) return false;
		if(!out.is_open()) return false;
	}
	return true;
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

	else if (command == "truncate") {
		if (validateTruncateCommand(command, option, argument) == SUCCESS) {
			return new TruncateCommand(argument);
		}
	}
	else if (command == "rm") {
		if (validateRmCommand(command, option, argument) == SUCCESS) {
			return new RmCommand(argument);
		}
	}
	else if (command == "tr") {
		InputStream* node = Stream::instance()->getFirst();
		std::string inputArg = node ? node->getArgument() : std::string();
		std::string whatArg = node ? node->getArgument2() : std::string();
		std::string withArg = node ? node->getArgument3() : std::string();
		if (validateTrCommand(command, option, inputArg, whatArg, withArg) == SUCCESS) {
			return new TrCommand(inputArg, whatArg, withArg);
		}
	}
	else if (command == "head") {
		// parse n from option
		if (validateHeadCommand(command, option, argument) == SUCCESS) {
			int n = std::stoi(option.substr(2));
			return new HeadCommand(argument, n);
		}
	}
    else {
		handleCommand(UNKNOWN_COMMAND, command, option, argument);
    }
    return nullptr;
}
