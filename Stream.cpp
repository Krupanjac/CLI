#include <string>
#include <iostream>
#include <fstream>

#include "Stream.h"

//Stream class
Stream* Stream::instance() {
	static Stream instance;
	return &instance;
}

void Stream::insert(InputStream *ei) {
	if (ei == nullptr) return;

	InputStream* nxt = first;
	InputStream* prv = nullptr;
	
	while (nxt != nullptr) {
		prv = nxt;
		nxt = nxt->getNext();
	}

	if (prv) prv->setNext(ei);
	else first = ei;

}



// Split pipeline by | separator and store in pipeline in reverse order
void Stream::split(std::string line) {
	std::string temp;

	// Traverse the string to split by '|'
	for (char c : line) {
		if (c == '|') {
			if (!temp.empty()) {
				pipeline.insert(pipeline.begin(), temp);
				temp.clear();
			}
		}
		else {
			temp += c; 
		}
	}

	// Push the last command to the front if temp is not empty
	if (!temp.empty()) {
		pipeline.insert(pipeline.begin(), temp);
	}
}

std::istream& operator>>(std::istream& in, Stream& stream) {
	std::string lineO;

	std::getline(in, lineO);

	if (lineO.empty()) {
		std::cerr << "\nError code 10 - Empty input" << std::endl;
		lineO = '?';
	}

	else if (lineO.length() > MAX_SIZE) {
		std::cerr << "\nError code 11 - Input size exceeds maximum size" << std::endl;
		lineO = '?';
	}

	stream.split(lineO);

	while(!stream.pipeline.empty()) {
		//Get the last element of the pipeline and remove it after
		std::string line = stream.pipeline.back();
		stream.pipeline.pop_back();

		//Create InputStream object
			InputStream* first = new InputStream(line);
			std::string arg = first->getArgument();

			//case 1: if argument is not empty and is quoted or a txt file
			if (arg.length() > 1) {
				bool isQuoted = arg.front() == '"' && arg.back() == '"';
				bool isTxtFile = arg.length() >= 4 && arg.substr(arg.length() - 4) == ".txt";

				if (isQuoted || isTxtFile) {
					if (isTxtFile && (arg.front() != '"' && arg.back() != '"')) {
						std::string command = first->getCommand();
						std::string option = first->getOption();
						delete first;
						first = nullptr;
						first = new FileStream(command, option, arg);
					}
					continue;
				}

	

				continue;
			}
			//case 2: if argument is empty and command is multiline
			else if (first->getCommand() == "echo" || first->getCommand() == "wc") {
				//Fetch from more lines
				if (first->getArgument().empty()) {
					while (std::getline(in, line)) {
						if (line == "") {
							arg += '"';
							first->setArgument(arg);
							break;
						}
						if (arg.length())
							arg += " " + line;

						else arg += '"' + line;
					}
					continue;
				}
				//first->setArgument(arg+"<");
				continue;

			}
			else continue;

		
	}

	return in;
}

void Stream::clear() {
	InputStream* current = first;
	while (current) {
		InputStream* next = current->getNext();
		delete current;
		current = next;
	}
	first = nullptr;
}

Stream::~Stream() {
	clear();
}


// InputStream Class
// Constructor to initialize input
InputStream::InputStream(std::string line) : next(nullptr) {
	read(line);
	Stream::instance()->insert(this);
}

// Default constructor
InputStream::InputStream() : next(nullptr) {}

std::string InputStream::find(const std::string& line, int& i) {
	std::string temp;
	int length = line.length();

	// Skip leading whitespaces
	while (i < length && std::isspace(line[i])) {
		i++;
	}

	// Check if the current token starts with a quote
	if (i < length && line[i] == '"') {
		int j = 0;
		while (i < length && j!=2) {
			if (line[0] == '"') j++;
			temp += line[i++];
		}
	}

	else {
		// Handle unquoted argument
		while (i < length && !std::isspace(line[i])) {
			temp += line[i++];
		}
	}

	// Skip trailing spaces after processing the token
	while (i < length && std::isspace(line[i])) {
		i++;
	}

	return temp;
}

std::string InputStream::removeWhiteSpaceTrail(std::string& str) {

	int i = str.length()-1;
	while (std::isspace(str[i])) {
		--i;
	}

	str = str.substr(0, i+1);

	return str;
}


void InputStream::read(const std::string& line) {
	int i = 0;  // Position in the input line (shifting string)
	//Fetch command

	setCommand(find(line, i)); 

	std::string temp = find(line, i);
	//Fetch if option exists
	if (!temp.empty() && temp[0] == '-') {
		setOption(temp);  
		temp = find(line, i); 
	}

	if (!temp.empty())
	temp = removeWhiteSpaceTrail(temp);

	setArgument(temp);  // Set argument if present


}




//FileStream class

FileStream::FileStream(const std::string& comm, const std::string& opt, const std::string& file) : InputStream() {
	setCommand(comm);
	setOption(opt);
	if (command == "echo" || command == "wc")
	readFromFile(file);
	else
		setArgument(file);


}


void FileStream::read(const std::string& line) {
	std::string arg = getArgument();
	arg += line + "\n";
	setArgument(arg);

}

void FileStream::readFromFile(const std::string& filePath) {
	try {
		std::ifstream file(filePath);
		if (!file.is_open()) {
			throw std::runtime_error("\nError code 5 - Could not open file: " + filePath);
		}

		std::string line;
		while (std::getline(file, line)) {
			read(line);
		}
		file.close();

		//add quotes to the argument
		if (getArgument().front() != '"' && getArgument().back() != '"') {
			std::string arg = getArgument();
			arg = '"' + arg + '"';
			setArgument(arg);
		}

	}
	catch (const std::exception& e) {
		std::cerr << "\nError code 6 - Error reading from file" << e.what() << std::endl;
	}
}