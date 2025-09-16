#include <iostream>
#include "Parser.h"

Parser::Parser() : stream(Stream::instance()) {

	std::cin >> *stream;
}


bool Parser::processOne() {
	InputStream* current = stream->getFirst();
	if (!current) return false;
	stream->setFirst(current->getNext());
	delete current;
	current = nullptr;
	return (stream->getFirst()!=nullptr);

}

bool Parser::processAll() {
	processOne();
	return(stream->getFirst() != nullptr);
}

Parser::~Parser() {
	stream = nullptr;
}