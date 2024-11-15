// main.cpp
#include <iostream>
#include <stdlib.h>

#include "ArgumentsParser.h"
#include "AuthReader.h"
#include "ImapClient.h"

int main(int argc, char* argv[]) {
    try {
        ArgumentsParser argumentsParser;
        ProgramOptions options = argumentsParser.parse(argc, argv);

        AuthReader authReader(options.authFile);
        AuthData authData = authReader.read();

        ImapClient imapClient(options);
        if (imapClient.run(authData) != 0) {
            return 1;
        }
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}