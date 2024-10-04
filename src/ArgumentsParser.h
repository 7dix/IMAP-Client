// ArgumentsParser.h
#ifndef ARGUMENTSPARSER_H
#define ARGUMENTSPARSER_H

#include <string>
#include <vector>
#include <map>

struct ProgramOptions {
    std::string server;
    int port = -1; // Not default port, used for checking changes
    bool useTLS = false; // Default no TLS
    std::string certFile;
    std::string certDir = "/etc/ssl/certs"; // Default cert dir
    bool onlyNewMessages = false; // Default all messages
    bool headersOnly = false; // Default full messages
    std::string authFile;
    std::string mailbox = "INBOX"; // Default mailbox
    std::string outputDir;
};

class ArgumentsParser {
public:
    ProgramOptions parse(int argc, char* argv[]);

private:
    void printUsage();
};

#endif // ARGUMENTSPARSER_H