// ImapParser.h
#ifndef IMAPPARSER_H
#define IMAPPARSER_H

#include <string>
#include <regex>
#include <iostream>
#include "ImapResponseRegex.h"

class ImapParser {
public:
    static int parseLoginResponse(const std::string &response);
};

#endif // IMAPPARSER_H