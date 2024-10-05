// main.cpp
#include <iostream>
#include <stdlib.h>

#include "ArgumentsParser.h"
#include "AuthReader.h"
#include "ImapClient.h"

int main(int argc, char* argv[]) {
    ArgumentsParser argumentsParser;
    ProgramOptions options = argumentsParser.parse(argc, argv);

    AuthReader authReader(options.authFile);
    AuthData authData = authReader.read();

    if (options.useTLS) {
        std::cout << "Načítám SSL.." << std::endl;
    }

    ImapClient imapClient(options);
    if (imapClient.state == ImapClientState::Error){
        imapClient.disconnect();
        return 1;
    }

    if (imapClient.login(authData) != 0){
        imapClient.disconnect();
        return 1;
    }
    if (imapClient.selectMailbox() != 0){
        imapClient.disconnect();
        return 1;
    }
    if (imapClient.fetchMessages() != 0){
        imapClient.disconnect();
        return 1;
    }
        
    imapClient.disconnect();
    return 0;
}