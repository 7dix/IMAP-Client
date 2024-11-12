// ImapParser.h
#ifndef IMAPPARSER_H
#define IMAPPARSER_H

#include <string>
#include <regex>
#include <iostream>
#include "ImapResponseRegex.h"

/**
 * @class ImapParser
 * @brief A class for parsing IMAP server responses.
 */
class ImapParser {
public:
    /**
     * @brief Parses the login response from the IMAP server.
     * @param response The response string from the server.
     * @return int Returns 0 if login is successful, otherwise returns 1.
     */
    static int parseLoginResponse(const std::string &response);

    /**
     * @brief Parses the select response from the IMAP server when selecting a mailbox.
     * @param response The response string from the server.
     * @return int Returns 0 if the mailbox is successfully selected, otherwise returns 1.
     */
    static int parseSelectResponse(const std::string &response);

    /**
     * @brief Parses the search response from the IMAP server to retrieve message IDs.
     * @param response The response string from the server.
     * @return std::vector<int> A vector containing the IDs of the messages found.
     */
    static std::vector<int> parseSearchResponse(const std::string &response);

    /**
     * @brief Parses the fetch response from the IMAP server to retrieve an email's contents.
     * @param response The response string from the server.
     * @return std::string The content of the email. Returns an empty string if fetching fails.
     */
    static std::string parseFetchResponse(const std::string &response);

    /**
     * @brief Checks if the response received contains the specified tag.
     * @param response The response string from the server.
     * @param tag The tag to search for in the response.
     * @return bool Returns true if the tag is found, otherwise false.
     */
    static bool checkResponseReceived(const std::string &response, const std::string &tag);
};

#endif // IMAPPARSER_H