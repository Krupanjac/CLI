#ifndef STREAM_H_
#define STREAM_H_
#define MAX_SIZE 512
#include <string>
#include <vector>

class InputStream;

//Class for handling DATA from Streams
class Stream {

public:

	//Singleton DP. Static. Returns singleton instance of this Class.
	static Stream* instance();

	//Inserts a new InputStream to be utilized.
	void insert(InputStream*);
	
	//virtual ~Stream();

	//Handle I Stream for Args
	virtual void setArgument(std::string);
	
	//Grab Args
	virtual std::string getArgument() const;


	//Handle I stream for Options
	virtual void setOption(std::string);

	//Grab Options
	virtual std::string getOption() const;

	//Handle I Stream for Commands
	virtual void setCommand(std::string);
	
	//Grab Commands
	virtual std::string getCommand() const;

	//Set first InputStream
	void setFirst(InputStream* first);

	//Grab first InputStream
	InputStream* getFirst() const;

	//clear singletron at end of lifetime
	void clear();

	//Split pipeline
	virtual void split(std::string);


	//Overloads std::input for processing multiple lines
	friend std::istream& operator>>(std::istream& in, Stream& stream);

protected:
	std::string command;
	std::string option;
	std::string argument;


	//Pipeline members (vector)
	std::vector<std::string> pipeline;

	Stream();
	~Stream();

private:
	
	//~Stream();
	InputStream* first;
	
};

inline Stream::Stream() : first(nullptr) {}



inline void Stream::setArgument(std::string arg) {
	argument = arg;
}

inline void Stream::setCommand(std::string comm) {
	command = comm;
}

inline void Stream::setOption(std::string opt) {
	option = opt;
}

inline std::string Stream::getArgument() const {
	return argument;
}

inline std::string Stream::getCommand() const {
	return command;
}

inline std::string Stream::getOption() const {
	return option;
}

inline void Stream::setFirst(InputStream* first) {
	this->first = first;
}

inline InputStream* Stream::getFirst() const {
	return first;
}


//I Stream class (keyboard)
class InputStream : public Stream {
public:
	//default constructor to initialize input
	InputStream();

	InputStream(std::string);
	//~InputStream();

	//Return next Stream input (if exists)
	InputStream* getNext();

	//Return last Stream input
	InputStream* getLast();

	//Set next chain in input
	void setNext(InputStream*);

	//helper function to remove whitespace in end of argument
	std::string removeWhiteSpaceTrail(std::string&);

protected:

	//Read from command line
	virtual void read(const std::string&);




private:
	//Handle line string from read
	std::string find(const std::string&, int&);


	InputStream* next;



};

inline InputStream* InputStream::getNext() {
	return this->next;
}

inline void InputStream::setNext(InputStream* nxt) {
	next = nxt;
}

inline InputStream* InputStream::getLast() {
	InputStream* tek = next;
	while (tek->next) tek = tek->next;
	return tek;
}

//F stream class (Files)
class FileStream : public InputStream {
public:
	//Constructor to copy data + handle File
	FileStream(const std::string&, const std::string&,const std::string& filePath);
	//Try read from file
	void readFromFile(const std::string& filePath);

protected:
	//Handle argument passed from File.
	void read(const std::string&) override;


	
};




#endif // !STREAM_H_
