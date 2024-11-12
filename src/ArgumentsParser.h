/**
 * @file ArgumentsParser.h
 * @brief Header file for the ArgumentsParser class and ProgramOptions struct.
 */

#ifndef ARGUMENTSPARSER_H
#define ARGUMENTSPARSER_H

#include <string>
#include <vector>
#include <map>

/**
 * @struct ProgramOptions
 * @brief Holds the program options parsed from command line arguments.
 */
struct ProgramOptions {
    std::string server; ///< Server address
    int port = -1; ///< Port number, default is -1 for checking changes
    bool useTLS = false; ///< Use TLS, default is false
    std::string certFile; ///< Certificate file path
    std::string certDir = "/etc/ssl/certs"; ///< Certificate directory, default is /etc/ssl/certs
    bool onlyNewMessages = false; ///< Download only new messages, default is false
    bool headersOnly = false; ///< Download only message headers, default is false
    std::string authFile; ///< Authentication file path
    std::string mailbox = "INBOX"; ///< Mailbox name, default is INBOX
    std::string outputDir; ///< Output directory path
};

/**
 * @class ArgumentsParser
 * @brief Parses command line arguments into ProgramOptions.
 */
class ArgumentsParser {
public:
    /**
     * @brief Parses command line arguments.
     * @param argc Argument count.
     * @param argv Argument vector.
     * @return Parsed program options.
     */
    ProgramOptions parse(int argc, char* argv[]);

private:
    /**
     * @brief Prints the usage information.
     */
    void printUsage();
};

#endif // ARGUMENTSPARSER_H