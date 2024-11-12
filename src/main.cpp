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

    ImapClient imapClient(options);


    // Main loop for the IMAP client
    while (imapClient.state != ImapClientState::Logout){
        switch (imapClient.state)
        {
        case ImapClientState::Disconnected:
            imapClient.connectImap();
            break;
        case ImapClientState::ConnectionEstabilished:
            imapClient.receiveGreeting();
            break;
        case ImapClientState::NotAuthenticated:
            imapClient.login(authData);
            break;
        case ImapClientState::Authenticated:
            imapClient.selectMailbox();
            break;
        case ImapClientState::SelectedMailbox:
            imapClient.fetchMessages();
            break;
        
        default:
            break;
        }
    }
    
    return 0;
}