// ArgumentsParser.h
#ifndef ARGUMENTSPARSER_H
#define ARGUMENTSPARSER_H

#include <string>
#include <vector>
#include <map>

struct ProgramOptions {
    std::string server;
    int port;
    bool useTLS;
    std::string certFile;
    std::string certDir;
    bool onlyNewMessages;
    bool headersOnly;
    std::string authFile;
    std::string mailbox;
    std::string outputDir;
};

class ArgumentsParser {
public:
    ProgramOptions parse(int argc, char* argv[]);

private:
    void printUsage();
};

#endif // ARGUMENTSPARSER_H