// ImapClient.cpp


// TODO delete unused includes
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

 
/**
 * @brief Constructs an ImapClient object and initializes the connection.
 * 
 * This constructor initializes the ImapClient object with the provided 
 * ProgramOptions, sets up the OpenSSL library, and attempts to connect 
 * to the IMAP server. If the connection fails, the constructor will 
 * return early.
 * 
 * @param options Reference to a ProgramOptions object containing 
 * configuration settings for the IMAP client.
 */
ImapClient::ImapClient(ProgramOptions &options)
    : options_(options), ssl_ctx_(nullptr), ssl_(nullptr) {

    // Initialize OpenSSL
    SSL_load_error_strings();   
    OpenSSL_add_ssl_algorithms();

    state = ImapClientState::Disconnected;
}

/**
 * @brief Destructor for the ImapClient class.
 *
 * This destructor ensures that the client is properly disconnected and that
 * any allocated SSL resources are freed. It performs the following actions:
 * - Calls the disconnect() method to ensure the client is disconnected.
 * - If an SSL connection is active (ssl_ is not null), it shuts down the SSL
 *   connection and frees the SSL structure.
 * - If an SSL context is active (ssl_ctx_ is not null), it frees the SSL context.
 * - Cleans up the OpenSSL library by calling EVP_cleanup().
 */
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

/**
 * @brief Connects to the IMAP server.
 *
 * This function establishes a connection to the IMAP server using the provided
 * server address and port from the options. It then performs a TLS handshake
 * if the useTLS option is enabled. The function updates the client's state
 * based on the success of the connection.
 *
 * The function performs the following steps:
 * - Calls establishConnection() to connect to the server.
 * - Calls TLSHandshake() if the useTLS option is enabled.
 * - Updates the client's state to ConnectionEstabilished if the connection is successful.
 * - Updates the client's state to Logout if the connection fails.
 */
void ImapClient::connectImap(){
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


/**
 * @brief Establishes a connection to the IMAP server.
 *
 * This function creates a socket and connects to the server using the provided
 * server address and port from the options. It uses DNS to resolve the server
 * address and attempts to establish a connection within a specified timeout.
 *
 * @return int Returns 0 on success, or 1 on failure.
 *
 * The function performs the following steps:
 * - Resolves the server address using DNS.
 * - Creates a socket for the connection.
 * - Attempts to connect to the server.
 * - Cleans up resources in case of failure.
 *
 * @note The function prints error messages to standard error in case of failure.
 */
int ImapClient::establishConnection(){
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

/**
 * @brief Performs a TLS handshake with the server.
 *
 * This function initializes the SSL context and performs a TLS handshake
 * with the server using the specified SSL context and socket. It also
 * loads the certificate file or directory if specified in the options.
 *
 * @return int Returns 0 on success, or 1 on failure.
 */
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

/**
 * Generates a unique tag for IMAP commands.
 * The tag is composed of the letter 'a' followed by a zero-padded command counter.
 * 
 * @return A string representing the generated tag.
 */
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



/**
 * @brief Logs in to the IMAP server using the provided authentication data.
 *
 * This function constructs and sends the LOGIN command to the server using the
 * provided username and password. It then receives and parses the server's response
 * to determine the success of the login operation.
 *
 * @param auth An AuthData object containing the username and password for authentication.
 */
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

/**
 * @brief Sends an IMAP command to the server.
 *
 * This function appends the CRLF sequence to the given command as required by the IMAP protocol,
 * and then sends the command to the server using either SSL or a regular socket.
 *
 * @param command The IMAP command to be sent.
 * @return int Returns 0 on success, or 1 if the command failed to send.
 */
int ImapClient::sendCommand(const std::string& command) {
    // Append CRLF to the command as per IMAP protocol
    std::string full_command = command + "\r\n";
    std::cout << "Odesílám: " << full_command << std::endl;
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

/**
 * @brief Receives and processes the greeting message from the IMAP server.
 *
 * This function waits for a greeting message from the IMAP server and updates
 * the client's state based on the received message. The possible states are:
 * - NotAuthenticated: If the server responds with an OK greeting.
 * - Authenticated: If the server responds with a PREAUTH greeting.
 * - Logout: If the server responds with a BYE greeting or if the response is not recognized.
 *
 * The function also outputs error messages to std::cerr if the server rejects
 * the connection or if the response cannot be recognized.
 */
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
            recved.find(currentTag + " NO") != std::string::npos ||
            recved.find(currentTag + " PREAUTH") != std::string::npos ||
            recved.find(currentTag + " BYE") != std::string::npos) {

            received = true;
        }
    }

    commandCounter++; // Increment after processing a complete command
    std::cout << "Přijato: " << response << std::endl;
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

    return 0; // Indicate success
}

int ImapClient::fetchMessages() {
    std::ostringstream command;
    if (options_.onlyNewMessages) {
        command << generateTag() << " SEARCH NEW" << std::endl;
    } else {
        command << generateTag() << " SEARCH ALL" << std::endl;
    }

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

    if (prepareOutputDir() != 0){
        return 1;
    }
    for (int id : messageIds) {
        std::string message = downloadMessage(id);
        if (message == ""){
            return 1;
        }
        if (saveMessage(message, id) != 0){
            std::cerr << "Nepodařilo se uložit email."<< std::endl;
            return 1;
        }
    }
    userInfo(messageIds.size());


    return 0;
}

void ImapClient::userInfo(const int messageCount){
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

// Extracts message IDs from the SEARCH response
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

// Downloads a message with a given ID
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

int ImapClient::saveMessage(const std::string &response, int id) {
    // TODO add head parsing without body
    // Use a regex to find the size of the message
    std::regex fetch_regex(R"(\* \d+ FETCH \(FLAGS \([^\)]+\) BODY\[[^\]]*\] \{(\d+)\})");
    std::smatch match;

    std::istringstream response_stream(response);
    std::string line;
    std::string message_content;
    size_t message_size = 0;
    bool reading_message = false;

    while (std::getline(response_stream, line)) {
        // Remove carriage returns if present
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }

        if (!reading_message) {
            // Check if the line matches the FETCH response
            if (std::regex_match(line, match, fetch_regex)) {
                // Read the message content

                message_size = std::stoul(match[1].str());
                reading_message = true;

                char *buffer = new char[message_size];
                response_stream.read(buffer, message_size);

                // Check if we read the correct amount of data
                std::streamsize bytes_read = response_stream.gcount();
                if (static_cast<size_t>(bytes_read) != message_size) {
                    delete[] buffer;
                    std::cerr << "Nepodařilo se načíst obsah emailu." << std::endl;
                    return 1;
                }

                // Convert buffer to string
                message_content.assign(buffer, message_size);
                delete[] buffer;
            }
        } else {
            // We have already read the message content
            break;
        }
    }

    // Check and create (if needed) the output directory
    mkdir(options_.outputDir.c_str(), 0775);

    // Create filename
    std::string filename = options_.outputDir + "/message_" + std::to_string(id) + ".eml";

    // Open the file in binary
    std::ofstream outfile(filename, std::ios::binary);
    if (!outfile.is_open()) {
        std::cerr << "Nepodařilo se nalézt cestu: " << filename << std::endl;
        return 1;
    }

    // Write the message content to the file
    outfile.write(message_content.data(), message_content.size());
    if (!outfile.good()) {
        std::cerr << "Nepodařilo se zapsat soubor: " << filename << std::endl;
        outfile.close();
        return 1;
    }

    // Close the file stream
    outfile.close();

    // Return 0 to indicate success
    return 0;
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