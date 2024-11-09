// ImapClient.h

#ifndef IMAPCLIENT_H
#define IMAPCLIENT_H

#include "ArgumentsParser.h"
#include "AuthReader.h"

#include "ImapParser.h"
#include "ImapResponseRegex.h"

#include "openssl/ssl.h"
#include "openssl/err.h"
#include "openssl/bio.h"


enum class ImapClientState {
    Disconnected,
    ConnectionEstabilished,
    NotAuthenticated,
    Authenticated,
    SelectedMailbox,
    Logout
};

class ImapClient {
public:
    ImapClient(ProgramOptions &options);
    ~ImapClient();
    void connectImap();
    void disconnect();
    void receiveGreeting();
    void login(AuthData auth);
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

    int establishConnection();
    int TLSHandshake();
    int sendCommand(const std::string& command);
    std::string receiveResponse();
    std::string recvData();
    std::string generateTag();
    std::vector<int> getMessageIdsFromResponse(const std::string &response);
    std::string downloadMessage(int id);
    int saveMessage(const std::string &message, int id);
    int prepareOutputDir();
    void userInfo(const int messageCount);
};

#endif // IMAPCLIENT_H