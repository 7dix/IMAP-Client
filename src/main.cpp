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
            if (imapClient.selectMailbox() != 0){
                return 1;
            }
            break;
        case ImapClientState::SelectedMailbox:
            if (imapClient.fetchMessages() != 0){
                return 1;
            }
            break;
        
        default:
            break;
        }
    }

    // switch (imapClient.state)
    // {
    // case ImapClientState::Disconnected:
    //     if (imapClient.connectImap() != 0){
    //         return 1;
    //     }
    //     break;
    // case ImapClientState::Logout:
    //     return 1;
    //     break;
    // case ImapClientState::NotAuthenticated:
    //     if (imapClient.login(authData) != 0){
    //         return 1;
    //     }
    //     break;
    
    // default:
    //     break;
    // } (imapClient.state == ImapClientState::Logout || imapClient.state == ImapClientState::Disconnected){
        
    // }

    // if (imapClient.login(authData) != 0){
    //     return 1;
    // }
    // if (imapClient.selectMailbox() != 0){
    //     return 1;
    // }
    // if (imapClient.fetchMessages() != 0){
    //     return 1;
    // }
    
    return 0;
}