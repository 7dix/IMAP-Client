// ImapClient.cpp

#include "ImapClient.h"
#include <stdlib.h>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cstring> 
#include <unistd.h>
 
ImapClient::ImapClient(ProgramOptions &options)
    : options_(options), ssl_ctx_(nullptr), ssl_(nullptr) {

    // Initialize OpenSSL
    SSL_load_error_strings();   
    OpenSSL_add_ssl_algorithms();

    state = ImapClientState::Connecting;
    if (connectImap() != 0) {
        state = ImapClientState::Error;
        return;
    }

    state = ImapClientState::Connected;
}

// Destructor
ImapClient::~ImapClient() {
    if (ssl_) {
        SSL_shutdown(ssl_);
        SSL_free(ssl_);
        ssl_ = nullptr;
    }
    if (ssl_ctx_) {
        SSL_CTX_free(ssl_ctx_);
        ssl_ctx_ = nullptr;
    }
    EVP_cleanup();
}

// handles connection to IMAP with login and TLS handling
int ImapClient::connectImap(){
    
    int conn = establishConnection();
    if (conn != 0){
        return 1;
    }

    if (options_.useTLS) {
        int hs = TLSHandshake();
        if (hs != 0){
            return 1;
        }
    }

    return 0;
}

// creates socket and connects to server with timeout
int ImapClient::establishConnection(){
    std::cout << "Vytvarim socket" << std::endl;

    // Find IP using DNS servers
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    std::cout << "Ziskavam IP.." << std::endl;

    int status = getaddrinfo(options_.server.c_str(), std::to_string(options_.port).c_str(), &hints, &res);
    if (status != 0) {
        std::cerr << "Chyba při získávání IP adresy serveru." << std::endl;
        return 1;
    }

    socket_ = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (socket_ < 0) {
        std::cerr << "Nepodařilo se otevřít socket pro připojení." << std::endl;
        return 1;
    }

    std::cout << "Pripojuji k serveru" << std::endl;

    // Connect to the server
    if (connect(socket_, res->ai_addr, res->ai_addrlen) == -1) {
        close(socket_); // Close the socket on failure
        freeaddrinfo(res);
        std::cerr << "Nepodařilo se připojit k serveru." << std::endl;
        return 1;
    }

    freeaddrinfo(res);

    std::cout << "Připojeno k IMAP, port: " << options_.port << std::endl;
    return 0;
}

// Sends a STARTTLS command and performs TLS handshake using OpenSSL
int ImapClient::TLSHandshake() {

    // Initialize SSL object for the connection
    ssl_ctx_ = SSL_CTX_new(SSLv23_client_method());
    if (!ssl_ctx_) {
        std::cerr << "Nepodařilo se vytvořit SSL kontext." << std::endl;
        return 1;
    }
    
    // Load the certificate file or dir if specified
    if (!options_.certFile.empty() || !options_.certDir.empty()) {
        if (SSL_CTX_load_verify_locations(ssl_ctx_, 
                                          options_.certFile.empty() ? nullptr : options_.certFile.c_str(), 
                                          options_.certDir.empty() ? nullptr : options_.certDir.c_str()) != 1) {
            std::cerr << "Nepodařilo se načíst SSL certifikáty." << std::endl;
            return 1;
        }
    }

    ssl_ = SSL_new(ssl_ctx_);
    SSL_set_fd(ssl_, socket_);

    // Perform SSL handshake
    if (SSL_connect(ssl_) <= 0) {
        std::cerr << "TLS handshake failed." << std::endl;
        ERR_print_errors_fp(stderr);
        return 1;
    }

    std::cout << "TLS handshake successful." << std::endl;
    return 0;
}

// Generates command tag
std::string ImapClient::generateTag(){
    std::ostringstream tagStream;
    std::string zeros;
    if (commandCounter < 10){
        zeros = "00";
    } else if (commandCounter < 100){
        zeros = "0";
    } else {
        zeros = "";
    }

    tagStream << "a" << zeros << commandCounter;
    return tagStream.str();
}


// Logs user into IMAP
int ImapClient::login(AuthData auth) {
    // Construct the LOGIN command
    std::ostringstream command;
    command << generateTag() << " LOGIN " << auth.username << " " << auth.password;

    // Send the LOGIN command
    if (sendCommand(command.str()) != 0) {
        std::cerr << "Nepodařilo se odeslat příkaz LOGIN." << std::endl;
        return 1;
    }

    // Receive the response from the server
    std::string response = receiveResponse();
    if (response == ""){
        return 1;
    }

    // Check the response for success (usually indicated by "OK")
    if (response.find("OK") == std::string::npos) {
        std::cerr << "Přihlášení se nezdařilo: " << response << std::endl;
        return 1; // Indicate failure
    }

    std::cout << "Úspěšně přihlášen!" << std::endl;
    return 0; // Indicate success
}

int ImapClient::sendCommand(const std::string& command) {
    // Append CRLF to the command as per IMAP protocol
    std::string full_command = command + "\r\n";
    std::cout << full_command;
    ssize_t bytes_sent;
    if (ssl_){
        bytes_sent = SSL_write(ssl_, full_command.c_str(), full_command.length());    
    } else {
        bytes_sent = send(socket_, full_command.c_str(), full_command.size(), 0);
    }
    if (bytes_sent < 0) {
        std::cerr << "Failed to send command to the server." << std::endl;
        return 1;
    }

    
    return 0;
}
std::string ImapClient::receiveResponse() {
    std::string currentTag = generateTag(); // Ensure this matches the last sent command
    std::string response;
    bool received = false;

    while (!received) {
        std::string recved = recvData();
        response += recved;

        // Check for completion of response
        if (recved.find(currentTag + " OK") != std::string::npos || 
            recved.find(currentTag + " BAD") != std::string::npos || 
            recved.find(currentTag + " NO") != std::string::npos) {

            std::cout << response << std::endl;
            received = true;
        }
    }

    commandCounter++; // Increment after processing a complete command
    return response;
}

std::string ImapClient::recvData() {
    char buffer[4096];
    int bytes_received;
    if (ssl_) {
        bytes_received = SSL_read(ssl_, buffer, sizeof(buffer) - 1);
    } else {
        bytes_received = recv(socket_, buffer, sizeof(buffer) - 1, 0);
    }
    
    if (bytes_received <= 0){
        throw std::runtime_error("Nepodařilo se přijmout data.");
    }
    buffer[bytes_received] = '\0';
    return std::string(buffer);
}

void ImapClient::disconnect() {
    if (socket_ != -1) {
        std::ostringstream command;
        command << generateTag() << " LOGOUT";
        // Send LOGOUT command and close the socket
        sendCommand(command.str());
        receiveResponse();
        close(socket_);
    }
    state = ImapClientState::Disconnected;
    std::cout << "Odpojeno od serveru." << std::endl;
}
 
int ImapClient::selectMailbox() {
    // Construct the SELECT command
    std::ostringstream command;
    command << generateTag() << " SELECT " << options_.mailbox;

    // Send the SELECT command
    if (sendCommand(command.str()) != 0){
        std::cerr << "Nepodařilo se odeslat příkaz SELECT." << std::endl;
        return 1;
    }

    // Receive the response from the server
    std::string response = receiveResponse();
    if (response == ""){
        std::cerr << "Zvolení schránky se nepodařilo, server neodpověděl."<< std::endl;
        return 1;
    }

    // Check the response for success (usually indicated by "OK")
    if (response.find("OK") == std::string::npos) {
        std::cerr << "Zvolení schránky se nepodařilo: " << response << std::endl;
        return 1; // Indicate failure
    }

    std::cout << "Schránka vybrána!" << std::endl;
    return 0; // Indicate success
}

int ImapClient::fetchMessages() {
    // TODO filtering based on parameters
    std::ostringstream command;
    command << generateTag() << " SEARCH ALL" << std::endl;

    // Send the SEARCH command
    if (sendCommand(command.str()) != 0){
        std::cerr << "Nepodařilo se odeslat příkaz SEARCH." << std::endl;
        return 1;
    }

    // Receive the response from the server
    std::string response = receiveResponse();
    if (response == ""){
        std::cerr << "Nepodařilo se najít emaily. Server neodpověděl."<< std::endl;
        return 1;
    }

    std::vector<int> messageIds = getMessageIdsFromResponse(response);
    std::cout << "Počet emailů: " << messageIds.size() << std::endl;

    for (int id : messageIds) {
        std::string message = downloadMessage(id);
        if (message == ""){
            std::cerr << "Nepodařilo se stáhnout email."<< std::endl;
            return 1;
        }
        std::cout << message << std::endl;
    }


    return 0;
}

std::vector<int> ImapClient::getMessageIdsFromResponse(const std::string &response) {
    std::vector<int> messageIds;
    std::istringstream responseStream(response);
    std::string line;
    while (std::getline(responseStream, line)) {
        if (line.find("* SEARCH") != std::string::npos) {
            std::istringstream lineStream(line);
            std::string search;
            int id;
            lineStream >> search >> search; // Skip "* SEARCH"
            while (lineStream >> id) {
                messageIds.push_back(id);
            }
        }
    }
    return messageIds;
}

std::string ImapClient::downloadMessage(int id) {
    std::ostringstream command;
    if (options_.headersOnly) {
        command << generateTag() << " FETCH " << id << " BODY[HEADER]";
    } else {
        command << generateTag() << " FETCH " << id << " BODY[]";
    }

    // Send the FETCH command
    if (sendCommand(command.str()) != 0){
        std::cerr << "Nepodařilo se odeslat příkaz FETCH." << std::endl;
        return "";
    }

    // Receive the response from the server
    std::string response = receiveResponse();
    if (response == ""){
        std::cerr << "Nepodařilo se stáhnout email. Server neodpověděl."<< std::endl;
        return "";
    }

    return response;
}

