// ImapClient.h
// author: Marek Tenora
// login: xtenor02

#ifndef IMAPCLIENT_H
#define IMAPCLIENT_H

#include "ArgumentsParser.h"
#include "AuthReader.h"
#include "FileHandler.h"

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
    /**
     * @brief Constructs an ImapClient object and initializes the connection.
     * 
     * Initializes the ImapClient object with the provided ProgramOptions,
     * sets up the OpenSSL library, and sets the initial client state.
     * 
     * @param options Reference to a ProgramOptions object containing 
     * configuration settings for the IMAP client.
     */
    ImapClient(ProgramOptions &options);

    /**
     * @brief Destructor for the ImapClient class.
     *
     * Ensures that the client is properly disconnected and that
     * any allocated SSL resources are freed.
     */
    ~ImapClient();

    /**
     * @brief Runs the IMAP client's main operation loop.
     *
     * Controls the IMAP client state machine through the following states:
     * - Disconnected -> Establishes initial connection
     * - ConnectionEstablished -> Receives server greeting
     * - NotAuthenticated -> Performs login
     * - Authenticated -> Selects mailbox
     * - SelectedMailbox -> Fetches messages
     * - Logout -> Disconnects from server
     *
     * @param authData Authentication credentials containing username and password
     * @return int Returns 0 on successful completion, 1 on error
     */
    int run(AuthData authData);

    /**
     * @brief Establishes a secure connection to the IMAP server.
     *
     * Performs the following steps:
     * 1. Creates a TCP socket connection to the server
     * 2. If TLS is enabled, performs TLS handshake
     * 3. Updates client state to ConnectionEstablished on success
     *
     * @throws ImapException If connection or TLS handshake fails
     */
    virtual void connectImap();

    /**
     * @brief Gracefully terminates the server connection.
     *
     * 1. Sends LOGOUT command to server if connected
     * 2. Closes socket and SSL connections
     * 3. Sets state to Disconnected
     */
    virtual void disconnect();

    /**
     * @brief Processes the server's initial greeting.
     *
     * Analyzes the greeting response and sets appropriate client state:
     * - OK -> NotAuthenticated
     * - PREAUTH -> Authenticated
     * - BYE -> Throws exception
     *
     * @throws ImapException If greeting is invalid or indicates server rejection
     */
    virtual void receiveGreeting();

    /**
     * @brief Authenticates with the IMAP server.
     *
     * Sends LOGIN command with credentials and validates server response.
     * Updates state to Authenticated on success.
     *
     * @param auth Authentication data containing username and password
     * @throws ImapException If authentication fails
     */
    virtual void login(AuthData auth);

    /**
     * @brief Opens the configured mailbox.
     *
     * Sends SELECT command for the mailbox specified in program options.
     * Updates state to SelectedMailbox on success.
     *
     * @throws ImapException If mailbox selection fails
     */
    virtual void selectMailbox();

    /**
     * @brief Downloads messages from the selected mailbox.
     *
     * 1. Searches for messages based on configured criteria
     * 2. Downloads new or all messages based on options
     * 3. Saves messages to output directory
     * 4. Updates state to Logout when complete
     *
     * @throws ImapException If message fetch operations fail
     * @throws FileException If message saving operations fail
     */
    virtual void fetchMessages();

    /**
     * @brief The current state of the IMAP client.
     */
    ImapClientState state = ImapClientState::Disconnected;

private:
    ProgramOptions options_;
    FileHandler* fileHandler;
    std::string username;
    int socket_;
    int commandCounter = 1;

    SSL_CTX* ssl_ctx_;
    SSL* ssl_; 

    /**
     * @brief Creates TCP socket connection to IMAP server.
     *
     * 1. Resolves server hostname using DNS
     * 2. Creates TCP socket
     * 3. Establishes connection to server
     *
     * @return int 0 on success, non-zero on failure
     * @throws ImapException If DNS resolution or socket operations fail
     */
    virtual int establishConnection();

    /**
     * @brief Sets up SSL/TLS encryption for the connection.
     *
     * 1. Initializes SSL context and certificate verification
     * 2. Creates SSL connection
     * 3. Performs SSL handshake
     *
     * @return int 0 on success, non-zero on failure
     * @throws ImapException If SSL initialization or handshake fails
     */
    virtual int TLSHandshake();

    /**
     * @brief Sends an IMAP command to the server.
     *
     * Handles both SSL and non-SSL connections.
     * Automatically appends CRLF to commands.
     *
     * @param command The IMAP command string to send
     * @return int 0 on success, non-zero on failure
     * @throws ImapException If send operation fails
     */
    virtual int sendCommand(const std::string &command);

    /**
     * @brief Receives complete IMAP server response.
     *
     * Collects response data until a complete tagged response is received.
     * Handles both SSL and non-SSL connections.
     *
     * @return std::string The complete server response
     * @throws ImapException If response cannot be fully received
     */
    virtual std::string receiveResponse();

    /**
     * @brief Low-level data receive operation with timeout.
     *
     * Implements timeout handling and supports both SSL and non-SSL connections.
     * Uses select() for timeout management.
     *
     * @return std::string The received data
     * @throws ImapException On timeout, disconnection, or read errors
     */
    virtual std::string recvData();

    /**
     * @brief Generates a unique tag for IMAP commands.
     *
     * The tag is composed of the letter 'A' followed by a command counter.
     *
     * @return A string representing the generated tag.
     */
    std::string generateTag();

    /**
     * @brief Parses message IDs from the server's response.
     *
     * @param response The response string received from the server.
     * @return A vector of message IDs extracted from the response.
     */
    std::vector<int> getMessageIdsFromResponse(const std::string &response);

    /**
     * @brief Downloads an email message from the IMAP server.
     *
     * Sends a UID FETCH command to retrieve the specified message.
     * Can fetch headers only or the entire message based on options.
     *
     * @param id The unique identifier of the email message to download.
     * @return A string containing the email message. Returns an empty string on failure.
     */
    virtual std::string downloadMessage(int id);

    /**
     * @brief Outputs information to the user about fetched messages.
     *
     * Provides a user-friendly message about the number of messages fetched.
     *
     * @param messageCount The number of messages that were fetched.
     */
    void userInfo(const int messageCount);
};

#endif // IMAPCLIENT_H