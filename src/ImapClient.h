// ImapClient.h

#ifndef IMAPCLIENT_H
#define IMAPCLIENT_H

#include "ArgumentsParser.h"
#include "AuthReader.h"
#include "openssl/ssl.h"
#include "openssl/err.h"
#include  "openssl/bio.h"


enum class ImapClientState {
    Disconnected,
    Connecting,
    Connected,
    Authenticating,
    Authenticated,
    Fetching,
    Error
};

class ImapClient {
public:
    ImapClient(ProgramOptions &options);
    ~ImapClient();
    void disconnect();
    int login(AuthData auth);
    int selectMailbox();
    int fetchMessages();
    std::string getMessages(int messageCount);
    ImapClientState state = ImapClientState::Disconnected;

private:
    ProgramOptions options_;
    int socket_;
    int commandCounter = 1;

    SSL_CTX* ssl_ctx_;
    SSL* ssl_; 

    int connectImap();
    int establishConnection();
    int TLSHandshake();
    int sendCommand(const std::string& command);
    std::string receiveResponse();
    std::string recvData();
    std::string generateTag();
};

#endif // IMAPCLIENT_H