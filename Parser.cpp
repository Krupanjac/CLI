#include <iostream>
#include "Parser.h"

Parser::Parser() {
	std::cin >> stream;
}

bool Parser::processOne() {
	if (stream.empty()) return false;
	return stream.advance();
}

bool Parser::processAll() {
	return processOne();
}