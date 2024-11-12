// ImapParser.cpp

#include "ImapParser.h"
#include <sstream>

/**
 * @brief Parses the login response from the IMAP server.
 * 
 * @param response The response string from the server.
 * @return int Returns 0 if login is successful, otherwise returns 1.
 */
int ImapParser::parseLoginResponse(const std::string &response) {
    std::istringstream responseStream(response);
    std::string line;
    std::smatch match;

    while (std::getline(responseStream, line)) {
        if (std::regex_search(line, match, LOGIN_OK)) {
            return 0;
        } else if (std::regex_search(line, match, LOGIN_NO)) {
            std::cerr << "Přihlášení se nezdařilo: " << match[1].str() << std::endl;
            return 1;
        } else if (std::regex_search(line, match, LOGIN_BAD)) {
            std::cerr << "Přihlášení se nezdařilo: " << match[1].str() << std::endl;
            return 1;
        }
    }

    std::cerr << "Přihlášení se nezdařilo." << std::endl;
    return 1;
}

/**
 * @brief Parses the select response from the IMAP server when selecting a mailbox.
 * 
 * @param response The response string from the server.
 * @return int Returns 0 if the mailbox is successfully selected, otherwise returns 1.
 */
int ImapParser::parseSelectResponse(const std::string &response) {
    std::istringstream responseStream(response);
    std::string line;
    std::smatch match;

    while (std::getline(responseStream, line)) {
        if (std::regex_search(line, match, SELECT_OK)) {
            return 0;
        } else if (std::regex_search(line, match, SELECT_NO)) {
            std::cerr << "Nepodařilo se vybrat složku: " << match[1].str() << std::endl;
            return 1;
        } else if (std::regex_search(line, match, SELECT_BAD)) {
            std::cerr << "Nepodařilo se vybrat složku: " << match[1].str() << std::endl;
            return 1;
        }
    }

    std::cerr << "Nepodařilo se vybrat složku." << std::endl;
    return 1;
}

/**
 * @brief Parses the search response from the IMAP server to retrieve message IDs.
 * 
 * @param response The response string from the server.
 * @return std::vector<int> A vector containing the IDs of the messages found.
 */
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

/**
 * @brief Parses the fetch response from the IMAP server to retrieve an email's contents.
 * 
 * @param response The response string from the server.
 * @return std::string The content of the email. Returns an empty string if fetching fails.
 */
std::string ImapParser::parseFetchResponse(const std::string &response) {
    std::istringstream responseStream(response);
    std::string line;
    std::smatch match;

    while (std::getline(responseStream, line)) {
        if (std::regex_search(line, match, FETCH_OK)) {

            // Match the actual message content
            std::regex fetch_regex(R"(\* \d+ FETCH .*?\{(\d+)\}\r?\n)");
            std::smatch match;

            if (std::regex_search(response, match, fetch_regex)) {
                // Extract the message size
                size_t message_size = std::stoul(match[1].str());

                // Find where the message content starts
                size_t message_start = match.position(0) + match.length(0);

                // Ensure we have enough data
                if (response.size() < message_start + message_size) {
                    std::cerr << "Nepodařilo se stáhnout email: chybí data." << std::endl;
                    return "";
                }

                // Extract the message content
                std::string message_content = response.substr(message_start, message_size);
                return message_content;
            }
            else{
                std::cerr << "Nepodařilo se stáhnout email: neznámá odpověď serveru." << std::endl;
                return "";
            }


            return 0;
        } 
        else if (std::regex_search(line, match, FETCH_NO)) {
            std::cerr << "Nepodařilo se stáhnout email: " << match[1].str() << std::endl;
            return "";
        } 
        else if (std::regex_search(line, match, FETCH_BAD)) {
            std::cerr << "Nepodařilo se stáhnout email: " << match[1].str() << std::endl;
            return "";
        }
    }

    return 0;
}

/**
 * @brief Checks if the response received contains the specified tag.
 * 
 * @param response The response string from the server.
 * @param tag The tag to search for in the response.
 * @return bool Returns true if the tag is found, otherwise false.
 */
bool ImapParser::checkResponseReceived(const std::string &response, const std::string &tag) {
    std::smatch match;
    if (std::regex_search(response, match, RESPONSE_TAG_REGEX)) {
        std::string found_tag = match[1].str();
        return tag.find(found_tag) != std::string::npos;
    }
    return false;
}
