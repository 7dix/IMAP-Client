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
#include <fstream>
#include <sys/stat.h>
#include <sys/types.h>
#include <regex>
#include <dirent.h>

ImapClient::ImapClient(ProgramOptions &options)
    : options_(options), ssl_ctx_(nullptr), ssl_(nullptr) {
    // Initialize OpenSSL
    SSL_load_error_strings();   
    OpenSSL_add_ssl_algorithms();

    state = ImapClientState::Disconnected;
}

ImapClient::~ImapClient() {
    disconnect();
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

void ImapClient::connectImap() {
    int conn = establishConnection();
    if (conn != 0){
        state = ImapClientState::Logout;
        return;
    }

    if (options_.useTLS) {
        int hs = TLSHandshake();
        if (hs != 0){
            state = ImapClientState::Logout;
            return;
        }
    }
    state = ImapClientState::ConnectionEstabilished;
    return;
}

int ImapClient::establishConnection() {
    // Find IP using DNS servers
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

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

    // Connect to the server
    if (connect(socket_, res->ai_addr, res->ai_addrlen) == -1) {
        close(socket_); // Close the socket on failure
        freeaddrinfo(res);
        std::cerr << "Nepodařilo se připojit k serveru." << std::endl;
        return 1;
    }

    freeaddrinfo(res);

    return 0;
}

int ImapClient::TLSHandshake() {
    // Initialize SSL object for the connection
    ssl_ctx_ = SSL_CTX_new(SSLv23_client_method());
    if (!ssl_ctx_) {
        std::cerr << "Nepodařilo se vytvořit SSL kontext." << std::endl;
        return 1;
    }
    
    // TODO check viz discord jestli je to spravne
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
    if (std::regex_search(recved, GREETING_OK)) {
        state = ImapClientState::NotAuthenticated;
        return;
    } else if (std::regex_search(recved, GREETING_PREAUTH)) {
        state = ImapClientState::Authenticated;
        return;
    } else if (std::regex_search(recved, GREETING_BYE)) {
        std::cerr << "Server odmítl spojení." << std::endl;
        state = ImapClientState::Logout;
        return;
    } else {
        std::cerr << "Nepodařilo se přijmout odpověď ze serveru." << std::endl;
        state = ImapClientState::Logout;
        return;
    }
}

void ImapClient::login(AuthData auth) {
    // Construct the LOGIN command
    std::ostringstream command;
    command << generateTag() << " LOGIN " << auth.username << " " << auth.password;

    // Send the LOGIN command
    if (sendCommand(command.str()) != 0) {
        std::cerr << "Nepodařilo se odeslat příkaz LOGIN." << std::endl;
        state = ImapClientState::Logout;
        return;
    }

    // Receive the response from the server
    std::string response = receiveResponse();

    // Parse the response to check for success login
    if (ImapParser::parseLoginResponse(response) != 0) {
        state = ImapClientState::Logout;
        return;
    }

    // Login successful
    state = ImapClientState::Authenticated;
    return;
}

void ImapClient::selectMailbox() {
    // Construct the SELECT command
    std::ostringstream command;
    command << generateTag() << " SELECT " << options_.mailbox;

    // Send the SELECT command
    if (sendCommand(command.str()) != 0){
        std::cerr << "Nepodařilo se odeslat příkaz SELECT." << std::endl;
        state = ImapClientState::Logout;
        return;
    }

    // Receive the response from the server
    std::string response = receiveResponse();
    if (ImapParser::parseSelectResponse(response) != 0){
        state = ImapClientState::Logout;
        return;
    }

    state = ImapClientState::SelectedMailbox;
    return;
}

void ImapClient::fetchMessages() {
    std::ostringstream command;
    if (options_.onlyNewMessages) {
        command << generateTag() << " SEARCH NEW" << std::endl;
    } else {
        command << generateTag() << " SEARCH ALL" << std::endl;
    }

    // Send the SEARCH command
    if (sendCommand(command.str()) != 0){
        std::cerr << "Nepodařilo se odeslat příkaz SEARCH." << std::endl;
        state = ImapClientState::Logout;
        return;
    }

    // Receive the response from the server
    std::string response = receiveResponse();

    std::vector<int> messageIds = ImapParser::parseSearchResponse(response);
    if (messageIds.empty()) {
        userInfo(0);
        state = ImapClientState::Logout;
        return;
    }

    if (prepareOutputDir() != 0){
        state = ImapClientState::Logout;
        return;
    }

    int count = 0;
    for (int id : messageIds) {
        std::string message = downloadMessage(id);
        if (message == ""){
            state = ImapClientState::Logout;
            return;
        }
        if (saveMessage(message, id) != 0){
            state = ImapClientState::Logout;
            return;
        }
        std::cout << "Staženo " << count << "/" << messageIds.size() << " zpráv." << std::endl;
        count++;
    }

    state = ImapClientState::Logout;
    userInfo(messageIds.size());
    return;
}

void ImapClient::userInfo(const int messageCount) {
    if (messageCount == 0){
        if (options_.onlyNewMessages){
            std::cout << "Žádné nové zprávy." << std::endl;
        } else {
            std::cout << "Žádné zprávy." << std::endl;
        }
    } else {
        std::string newText;
        std::string dledText;
        std::string messText;
        std::string headText = "(pouze hlavičky) ";
        if (messageCount >= 5){
            messText = " zpráv ";
            newText = " nových";
            dledText = "Staženo ";
        } else if ( messageCount == 1){
            messText = " zpráva ";
            newText = " nová";
            dledText = "Stažena ";
        } else {
            messText = " zprávy ";
            newText = " nové";
            dledText = "Staženy ";
        }

        if (!options_.onlyNewMessages){
            newText = "";
        }
        if (!options_.headersOnly){
            headText = "";
        }

        std::cout << dledText << messageCount << newText << messText << headText << "ze schránky " << options_.mailbox << "." <<  std::endl;
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
        std::cerr << "Failed to send command to the server." << std::endl;
        return 1;
    }

    return 0;
}

std::string ImapClient::receiveResponse() {
    std::string currentTag = generateTag();
    std::string response;
    bool received = false;
    int maxTries = 5000;

    while (!received && maxTries > 0) {
        std::string recved = recvData();
        response += recved;

        // Check for completion of response
        if (ImapParser::checkResponseReceived(recved, currentTag)) {
            received = true;
        }
        maxTries--;

        if (maxTries == 0) {
            std::cerr << "Nepodařilo se přijmout kompletní odpověď ze serveru." << std::endl;
            return "";
        }
    }

    commandCounter++;
    return response;
}

std::string ImapClient::recvData() {
    char buffer[4096];
    int bytes_received;
    if (ssl_) {
        bytes_received = SSL_read(ssl_, buffer, sizeof(buffer) - 1);
        if (bytes_received <= 0) {
            int ssl_error = SSL_get_error(ssl_, bytes_received);
            if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE) {
                // Recursively call recvData to repeat the read operation
                return recvData();
            } else if (ssl_error == SSL_ERROR_ZERO_RETURN) {
                std::cerr << "Server ukončil spojení." << std::endl;
                return "";
            } else {
                std::cerr << "Nastal problém s SSL: " << ssl_error << std::endl;
                return "";
            }
        }
    } else {
        bytes_received = recv(socket_, buffer, sizeof(buffer) - 1, 0);
        if (bytes_received <= 0) {
            std::cerr << "Nepodařilo se přijmout data ze serveru." << std::endl;
            return "";
        }
    }
    buffer[bytes_received] = '\0';
    return std::string(buffer);
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
    std::string message = ImapParser::parseFetchResponse(response);

    return message;
}

int ImapClient::saveMessage(const std::string &message_content, int id) {
    if (!message_content.empty()) {
        // Check and create (if needed) the output directory
        mkdir(options_.outputDir.c_str(), 0775);

        // Create filename
        std::string filename = options_.outputDir + "/message_" + std::to_string(id) + ".eml";

        // Open the file in binary mode
        std::ofstream outfile(filename, std::ios::binary);
        if (!outfile.is_open()) {
            std::cerr << "Nepodařilo se otevřít soubor: " << filename << std::endl;
            return 1;
        }

        // Write the message content to the file
        outfile.write(message_content.data(), message_content.size());
        if (!outfile.good()) {
            std::cerr << "Nepodařilo se zapsat do souboru: " << filename << std::endl;
            outfile.close();
            return 1;
        }

        // Close the file stream
        outfile.close();

        // Return 0 to indicate success
        return 0;
    } else {
        return 1;
    }
}

int ImapClient::prepareOutputDir() {
    struct stat info;
    if (stat(options_.outputDir.c_str(), &info) == 0 && (info.st_mode & S_IFDIR)) {
        // Directory exists
        DIR *dir = opendir(options_.outputDir.c_str());
        if (dir) {
            struct dirent *entry;
            while ((entry = readdir(dir)) != nullptr) {
                if (entry->d_name[0] != '.') { // Skip "." and ".."
                    std::string filePath = options_.outputDir + "/" + entry->d_name;
                    if (remove(filePath.c_str()) != 0) {
                        std::cerr << "Nepodařilo se smazat soubor: " << filePath << std::endl;
                        closedir(dir);
                        return 1;
                    }
                }
            }
            closedir(dir);
        } else {
            std::cerr << "Nepodařilo se otevřít adresář: " << options_.outputDir << std::endl;
            return 1;
        }
    } else {
        // Directory does not exist, create it
        if (mkdir(options_.outputDir.c_str(), 0775) != 0) {
            std::cerr << "Nepodařilo se vytvořit adresář: " << options_.outputDir << std::endl;
            return 1;
        }
    }
    return 0;
}