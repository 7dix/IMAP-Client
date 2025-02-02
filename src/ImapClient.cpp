// ImapClient.cpp
// author: Marek Tenora
// login: xtenor02

#include "ImapClient.h"
#include "ImapException.h"
#include "FileHandler.h"
#include "FileException.h"
#include "ImapResponseRegex.h"
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstring> 
#include <unistd.h>
#include <fstream>
#include <regex>
#include <dirent.h>
#include <sys/select.h>
#include <errno.h>

ImapClient::ImapClient(ProgramOptions &options)
    : options_(options), ssl_ctx_(nullptr), ssl_(nullptr) {
    // Initialize OpenSSL
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    // Set the initial state
    state = ImapClientState::Disconnected;

    // Initialize FileHandler
    fileHandler = new FileHandler(options_.outputDir);
}

ImapClient::~ImapClient() {
    if (socket_ != -1) {
        close(socket_);
    }
    if (ssl_) { 
        SSL_shutdown(ssl_);
        SSL_free(ssl_);
        ssl_ = nullptr;
    }
    if (ssl_ctx_) {
        SSL_CTX_free(ssl_ctx_);
        ssl_ctx_ = nullptr;
    }

    delete fileHandler;
    EVP_cleanup();
}

int ImapClient::run(AuthData authData) {
    username = authData.username;
    while (state != ImapClientState::Logout){
        try {
            switch (state)
            {
            case ImapClientState::Disconnected:
                connectImap();
                break;
            case ImapClientState::ConnectionEstabilished:
                receiveGreeting();
                break;
            case ImapClientState::NotAuthenticated:
                login(authData);
                break;
            case ImapClientState::Authenticated:
                selectMailbox();
                break;
            case ImapClientState::SelectedMailbox:
                fetchMessages();
                break;
            
            default:
                break;
            }
        } catch (const ImapException& e) {
            std::cerr << "IMAP error: " << e.what() << std::endl;
            state = ImapClientState::Logout;
            disconnect();
            return 1;
        } catch (const FileException& e) {
            std::cerr << "File error: " << e.what() << std::endl;
            state = ImapClientState::Logout;
            disconnect();
            return 1;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            state = ImapClientState::Logout;
            disconnect();
            return 1;
        }
    }
    disconnect();
    return 0;
}


void ImapClient::connectImap() {
    int conn = establishConnection();
    if (conn != 0){
        throw ImapException("Failed to establish connection");
    }

    if (options_.useTLS) {
        int hs = TLSHandshake();
        if (hs != 0){
            throw ImapException("Failed to establish TLS connection");
        }
    }
    state = ImapClientState::ConnectionEstabilished;
    return;
}

int ImapClient::establishConnection() {
    struct addrinfo hints{}, *res = nullptr, *p = nullptr;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Resolve server address
    int status = getaddrinfo(options_.server.c_str(), std::to_string(options_.port).c_str(), &hints, &res);
    if (status != 0) {
        throw ImapException("Failed to resolve server address: " + std::string(gai_strerror(status)));
    }

    // Attempt to connect using each address info result
    for (p = res; p != nullptr; p = p->ai_next) {

        socket_ = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_ < 0) {
            continue;
        }

        if (::connect(socket_, p->ai_addr, p->ai_addrlen) == 0) {
            break;
        }

        close(socket_);
        socket_ = -1;
    }

    freeaddrinfo(res);

    if (socket_ < 0) {
        throw ImapException("Failed to connect to server on any resolved address");
    }

    return 0;
}

int ImapClient::TLSHandshake() {
    // Initialize SSL object for the connection
    ssl_ctx_ = SSL_CTX_new(SSLv23_client_method());
    if (!ssl_ctx_) {
        throw ImapException("Failed to create SSL context");
    }
    
    // TODO check viz discord jestli je to spravne
    // Load the certificate file or dir if specified
    if (!options_.certFile.empty() || !options_.certDir.empty()) {
        if (SSL_CTX_load_verify_locations(ssl_ctx_, 
                                          options_.certFile.empty() ? nullptr : options_.certFile.c_str(), 
                                          options_.certDir.empty() ? nullptr : options_.certDir.c_str()) != 1) {
            throw ImapException("Failed to load SSL certificates");
        }
    }

    ssl_ = SSL_new(ssl_ctx_);
    SSL_set_fd(ssl_, socket_);

    // Perform SSL handshake
    if (SSL_connect(ssl_) <= 0) {
        throw ImapException("Failed to establish TLS connection");
        ERR_print_errors_fp(stderr);
    }

    return 0;
}

void ImapClient::disconnect() {
    // Send LOGOUT command if connected
    if (socket_ != -1) {
        std::ostringstream command;
        command << generateTag() << " LOGOUT";
        // Send LOGOUT command and close the socket
        sendCommand(command.str());
        receiveResponse();
        close(socket_);
    }
    state = ImapClientState::Disconnected;
}

void ImapClient::receiveGreeting() {
    std::string recved = recvData();
    int greetingRes = ImapParser::parseGreetingResponse(recved);
    if (greetingRes == 1){
        state = ImapClientState::NotAuthenticated;
    } else if (greetingRes == 0){
        state = ImapClientState::Authenticated;
    } else {
        throw ImapException("Failed to receive greeting from server");
    }
}

void ImapClient::login(AuthData auth) {
    std::ostringstream command;
    command << generateTag() << " LOGIN " << auth.username << " " << auth.password;

    if (sendCommand(command.str()) != 0) {
        throw ImapException("Failed to send LOGIN command");
    }

    std::string response = receiveResponse();

    ImapParser::parseLoginResponse(response);

    state = ImapClientState::Authenticated;
}

void ImapClient::selectMailbox() {
    std::ostringstream command;
    command << generateTag() << " SELECT " << options_.mailbox;

    if (sendCommand(command.str()) != 0) {
        throw ImapException("Failed to send SELECT command");
    }

    std::string response = receiveResponse();
    ImapParser::parseSelectResponse(response);

    int uidValidity = ImapParser::parseUIDValidity(response);
    int uidCheck = fileHandler->checkMailboxUIDValidity(username, options_.mailbox, uidValidity);
    if (uidCheck == -1){
        throw FileHandler("Failed to check UIDVALIDITY");
    }

    state = ImapClientState::SelectedMailbox;
}

void ImapClient::fetchMessages() {
    std::ostringstream command;
    if (options_.onlyNewMessages) {
        command << generateTag() << " UID SEARCH NEW";
    } else {
        command << generateTag() << " UID SEARCH ALL";
    }

    if (sendCommand(command.str()) != 0) {
        throw ImapException("Failed to send SEARCH command");
    }

    std::string response = receiveResponse();
    std::vector<int> messageIds = ImapParser::parseSearchResponse(response);

    int count = 0;
    std::vector<int> downloadedMessages;

    for (int id : messageIds){
        int messageInfo = fileHandler->isMessageAlreadyDownloaded(id, username, options_.mailbox);
        if (messageInfo == 1 || (messageInfo == 2 && options_.headersOnly)){
            downloadedMessages.push_back(id);
        } else if (messageInfo == -1){
            throw ImapException("Failed to determine message status");
        }
    }

    if (messageIds.size() != 0){

        for (int id : messageIds) {
            if (std::find(downloadedMessages.begin(), downloadedMessages.end(), id) != downloadedMessages.end()){
                continue;
            }
            // Download the message
            std::string message = downloadMessage(id);
            if (message.empty()) {
                throw ImapException("Failed to download message");
            }
            fileHandler->saveMessage(message, id, username, options_.mailbox);
            
            count++;
        }
    }

    state = ImapClientState::Logout;
    userInfo(messageIds.size() - downloadedMessages.size());
    return;
}

void ImapClient::userInfo(const int messageCount) {
    if (messageCount == 0){
        if (options_.onlyNewMessages){
            std::cout << "No new messages downloaded." << std::endl;
        } else {
            std::cout << "No messages downloaded." << std::endl;
        }
    } else {
        std::string newText;
        std::string dledText;
        std::string messText;
        std::string headText = "(headers only) ";
        if (messageCount >= 5){
            messText = " messages ";
            newText = " new";
            dledText = "Downloaded ";
        } else if ( messageCount == 1){
            messText = " message ";
            newText = " new";
            dledText = "Downloaded ";
        } else {
            messText = " messages ";
            newText = " new";
            dledText = "Downloaded ";
        }

        if (!options_.onlyNewMessages){
            newText = "";
        }
        if (!options_.headersOnly){
            headText = "";
        }

        std::cout << dledText << messageCount << newText << messText << headText << "from mailbox " << options_.mailbox << "." <<  std::endl;
    }
}

std::string ImapClient::generateTag() {
    std::ostringstream tagStream;

    tagStream << "A" << commandCounter;
    return tagStream.str();
}

int ImapClient::sendCommand(const std::string& command) {
    // Append CRLF to the command as per IMAP protocol
    std::string full_command = command + "\r\n";

    ssize_t bytes_sent;
    if (ssl_){
        bytes_sent = SSL_write(ssl_, full_command.c_str(), full_command.length());    
    } else {
        bytes_sent = send(socket_, full_command.c_str(), full_command.size(), 0);
    }
    if (bytes_sent < 0) {
        throw ImapException("Failed to send command to server");
    }

    return 0;
}

std::string ImapClient::receiveResponse() {
    std::string currentTag = generateTag();
    std::string response;
    bool received = false;
    int maxTries = 1000;

    while (!received && maxTries > 0) {
        std::string recved = recvData();
        response += recved;

        // Check for completion of response
        if (ImapParser::checkResponseReceived(recved, currentTag)) {
            received = true;
        }
        maxTries--;

        if (maxTries == 0) {
            throw ImapException("Failed to receive complete response from server");
        }
    }

    commandCounter++;
    return response;
}

std::string ImapClient::recvData() {
    char buffer[4096];
    int bytes_received = 0;
    const int timeout_seconds = 30;

    while (true) {
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(socket_, &read_fds);
        int max_fd = socket_ + 1;

        struct timeval timeout;
        timeout.tv_sec = timeout_seconds;
        timeout.tv_usec = 0;

        int select_result;
        if (ssl_) {
            // For SSL, use SSL_pending to check if there's data to read
            if (SSL_pending(ssl_) > 0) {
                select_result = 1; // Data is already pending
            } else {
                select_result = select(max_fd, &read_fds, NULL, NULL, &timeout);
            }
        } else {
            // For non-SSL, use select directly
            select_result = select(max_fd, &read_fds, NULL, NULL, &timeout);
        }

        if (select_result > 0) {
            if (FD_ISSET(socket_, &read_fds)) {
                if (ssl_) {
                    bytes_received = SSL_read(ssl_, buffer, sizeof(buffer) - 1);
                } else {
                    bytes_received = recv(socket_, buffer, sizeof(buffer) - 1, 0);
                }

                if (bytes_received > 0) {
                    buffer[bytes_received] = '\0';

                    // Return the received data
                    return std::string(buffer);

                } else if (bytes_received == 0) {
                    // Connection closed by the server
                    throw ImapException("Server closed connection");
                } else {
                    if (ssl_) {
                        int ssl_error = SSL_get_error(ssl_, bytes_received);
                        if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE) {
                            // The operation did not complete; retry
                            continue;
                        } else if (ssl_error == SSL_ERROR_ZERO_RETURN) {
                            throw ImapException("Server closed connection");
                        } else {
                            // Print SSL error details for debugging
                            ERR_print_errors_fp(stderr);
                            throw ImapException("SSL connection problem occurred");
                        }
                    } else {
                        if (errno == EWOULDBLOCK || errno == EAGAIN) {
                            // No data available; retry
                            continue;
                        } else {
                            throw ImapException("Failed to receive data from server");
                        }
                    }
                }
            }
        } else if (select_result == 0) {
            // Timeout occurred
            throw ImapException("Timeout while waiting for data from server");
        } else {
            // select() error
            if (errno == EINTR) {
                // Interrupted by signal, retry
                continue;
            }
            throw ImapException("Failed to receive data from server");
        }
    }
}


std::string ImapClient::downloadMessage(int id) {
    std::ostringstream command;
    if (options_.headersOnly) {
        command << generateTag() << " UID FETCH " << id << " BODY[HEADER]";
    } else {
        command << generateTag() << " UID FETCH " << id << " BODY[]";
    }

    // Send the FETCH command
    if (sendCommand(command.str()) != 0){
        throw ImapException("Failed to send FETCH command");
    }

    std::string response;
    try {
        // Receive the response from the server
        response = receiveResponse();
    } catch (const ImapException& e) {
        // If the message could not be downloaded, try sending command again
        std::cerr << "Failed to download message from server. Retrying one more time..." << std::endl;
        if (sendCommand(command.str()) != 0){
            throw ImapException("Failed to send FETCH command");
        }
        response = receiveResponse();
    }
    std::string message = ImapParser::parseFetchResponse(response);

    return message;
}