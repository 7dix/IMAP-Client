// ImapParser.h
// author: Marek Tenora
// login: xtenor02

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
     * @brief Parses the greeting response from the IMAP server.
     * @param response The response string from the server.
     * @return int Returns 1 if the greeting is OK, 0 if it is PREAUTH, and throws an exception otherwise.
     */
    static int parseGreetingResponse(const std::string &response);

    /**
     * @brief Parses the login response from the IMAP server.
     * @param response The response string from the server.
     */
    static void parseLoginResponse(const std::string &response);

    /**
     * @brief Parses the select response from the IMAP server when selecting a mailbox.
     * @param response The response string from the server.
     */
    static void parseSelectResponse(const std::string &response);

    /** 
     * @brief Parses the UIDVALIDITY response from the IMAP server.
     * @param response The response string from the server.
     * @return int The UIDVALIDITY value.
     */
    static int parseUIDValidity(const std::string &response);

    /**
     * @brief Parses the search response from the IMAP server to retrieve message IDs.
     * @param response The response string from the server.
     * @return std::vector<int> A vector containing the IDs of the messages found.
     */
    static std::vector<int> parseSearchResponse(const std::string &response);

    /**
     * @brief Parses the fetch response from the IMAP server to retrieve an email's contents.
     * @param response The response string from the server.
     * @return std::string The content of the email.
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