// ImapParser.cpp
// author: Marek Tenora
// login: xtenor02

#include "ImapParser.h"
#include "ImapException.h"
#include <sstream>


int ImapParser::parseGreetingResponse(const std::string &response) {
    if (std::regex_search(response, GREETING_OK)) {
        return 1;
    } else if (std::regex_search(response, GREETING_PREAUTH)) {
        return 0;
    } else if (std::regex_search(response, GREETING_BYE)) {
        throw ImapException("Server refused connection.");
    } else {
        throw ImapException("Failed to receive response from server.");
    }
}

void ImapParser::parseLoginResponse(const std::string &response) {
    std::istringstream responseStream(response);
    std::string line;
    std::smatch match;

    while (std::getline(responseStream, line)) {
        if (std::regex_search(line, match, LOGIN_OK)) {
            return;
        } else if (std::regex_search(line, match, LOGIN_NO)) {
            throw ImapException("Login failed: " + match[1].str());
        } else if (std::regex_search(line, match, LOGIN_BAD)) {
            throw ImapException("Login failed: " + match[1].str());
        }
    }

    throw ImapException("Login failed.");
}

void ImapParser::parseSelectResponse(const std::string &response) {
    std::istringstream responseStream(response);
    std::string line;
    std::smatch match;

    while (std::getline(responseStream, line)) {
        if (std::regex_search(line, match, SELECT_OK)) {
            return;
        } else if (std::regex_search(line, match, SELECT_NO)) {
            throw ImapException("Failed to select mailbox: " + match[1].str());
        } else if (std::regex_search(line, match, SELECT_BAD)) {
            throw ImapException("Failed to select mailbox: " + match[1].str());
        }
    }

    throw ImapException("Failed to select mailbox.");
}

int ImapParser::parseUIDValidity(const std::string &response) {
    std::istringstream responseStream(response);
    std::string line;
    std::smatch match;
    std::regex uidValidityRegex(R"(OK \[UIDVALIDITY (\d+)\])");

    while (std::getline(responseStream, line)) {
        if (std::regex_search(line, match, uidValidityRegex)) {
            return std::stoi(match[1].str());
        }
    }
    
    throw ImapException("Failed to obtain UIDVALIDITY.");
}



std::vector<int> ImapParser::parseSearchResponse(const std::string &response) {
    std::vector<int> messageIds;
    std::istringstream responseStream(response);
    std::string line;

    while (std::getline(responseStream, line)) {
        if (line.find("* SEARCH") != std::string::npos) {
            std::istringstream lineStream(line);
            std::string search;
            int id;
            lineStream >> search >> search; // Skip "* SEARCH"
            while (lineStream >> id) {
                messageIds.push_back(id);
            }
        }
    }

    return messageIds;
}

std::string ImapParser::parseFetchResponse(const std::string &response) {
    std::istringstream responseStream(response);
    std::string line;
    std::smatch match;

    while (std::getline(responseStream, line)) {
        if (std::regex_search(line, match, FETCH_OK)) {

            std::smatch match;

            if (std::regex_search(response, match, FETCH_MESSAGE)) {
                // Extract the message size
                size_t message_size = std::stoul(match[1].str());

                // Find where the message content starts
                size_t message_start = match.position(0) + match.length(0);

                // Ensure we have enough data
                if (response.size() < message_start + message_size) {
                    throw ImapException("Failed to download email: data missing.");
                }

                // Extract the message content
                std::string message_content = response.substr(message_start, message_size);
                return message_content;
            }
            else{
                throw ImapException("Failed to download email: unknown server response.");
            }
        } 
        else if (std::regex_search(line, match, FETCH_NO)) {
            throw ImapException("Failed to download email: " + match[1].str());
        } 
        else if (std::regex_search(line, match, FETCH_BAD)) {
            throw ImapException("Failed to download email: " + match[1].str());
        }
    }

    throw ImapException("Failed to parse server response.");
}

bool ImapParser::checkResponseReceived(const std::string &response, const std::string &tag) {
    std::smatch match;
    if (std::regex_search(response, match, RESPONSE_TAG_REGEX)) {
        std::string found_tag = match[1].str();
        return tag.find(found_tag) != std::string::npos;
    }
    return false;
}
