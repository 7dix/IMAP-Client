// ImapParser.cpp

#include "ImapParser.h"

int ImapParser::parseLoginResponse(const std::string &response) {
    std::smatch match;
    if (std::regex_search(response, match, LOGIN_OK)) {
        return 0;
    } else {
        if (std::regex_search(response, match, LOGIN_NO)) {
            std::cerr << "Přihlášení se nezdařilo: " << match[1].str() << std::endl;
        } else if (std::regex_search(response, match, LOGIN_BAD)) {
            std::cerr << "Přihlášení se nezdařilo: " << match[1].str() << std::endl;
        } else {
            std::cerr << "Přihlášení se nezdařilo." << std::endl;
        }
        return 1;
    }
}