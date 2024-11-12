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
     * @brief Connects to the IMAP server.
     *
     * Establishes a connection to the IMAP server using the provided
     * server address and port from the options. Performs a TLS handshake
     * if the useTLS option is enabled. Updates the client's state
     * based on the success of the connection.
     */
    void connectImap();

    /**
     * @brief Disconnects the IMAP client from the server.
     *
     * Sends the LOGOUT command and closes the connection. Updates
     * the client's state to Disconnected.
     */
    void disconnect();

    /**
     * @brief Receives the initial greeting message from the IMAP server.
     *
     * Waits for a greeting message from the IMAP server and updates
     * the client's state based on the received message.
     */
    void receiveGreeting();

    /**
     * @brief Logs in to the IMAP server using the provided authentication data.
     *
     * Sends the LOGIN command with the provided username and password.
     * Receives and parses the server's response to determine
     * the success of the login operation.
     *
     * @param auth An AuthData object containing the username and password for authentication.
     */
    void login(AuthData auth);

    /**
     * @brief Selects the mailbox specified in the program options.
     *
     * Constructs and sends the SELECT command to select the mailbox.
     * Updates the client's state based on the server's response.
     */
    void selectMailbox();

    /**
     * @brief Fetches messages from the selected mailbox.
     *
     * Searches for messages based on options, downloads them,
     * and saves them to the specified output directory.
     */
    void fetchMessages();

    /**
     * @brief Retrieves messages from the server.
     *
     * @param messageCount The number of messages to retrieve.
     * @return A string containing the messages.
     */
    std::string getMessages(int messageCount);

    /**
     * @brief The current state of the IMAP client.
     */
    ImapClientState state = ImapClientState::Disconnected;

private:
    ProgramOptions options_;
    int socket_;
    int commandCounter = 1;

    SSL_CTX* ssl_ctx_;
    SSL* ssl_; 

    /**
     * @brief Establishes a connection to the IMAP server.
     *
     * Creates a socket and connects to the server using the provided
     * server address and port from the options. Uses DNS to resolve the server
     * address and attempts to establish a connection.
     *
     * @return int Returns 0 on success, or 1 on failure.
     */
    int establishConnection();

    /**
     * @brief Performs a TLS handshake with the server.
     *
     * Initializes the SSL context and performs a TLS handshake
     * with the server. Loads the certificate file or directory
     * if specified in the options.
     *
     * @return int Returns 0 on success, or 1 on failure.
     */
    int TLSHandshake();

    /**
     * @brief Sends an IMAP command to the server.
     *
     * Appends the CRLF sequence to the given command and sends
     * it to the server using SSL or a regular socket.
     *
     * @param command The IMAP command to be sent.
     * @return int Returns 0 on success, or 1 if the command failed to send.
     */
    int sendCommand(const std::string &command);

    /**
     * @brief Receives and processes the response from the IMAP server.
     *
     * Continuously receives data until a complete response is detected.
     * Increments the command counter after processing.
     *
     * @return A string containing the complete response from the server.
     */
    std::string receiveResponse();

    /**
     * @brief Receives data from the IMAP server.
     *
     * Reads data from the server into a buffer and returns it as a string.
     * Supports both SSL and non-SSL connections.
     *
     * @return A string containing the data received from the server.
     */
    std::string recvData();

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
     * Sends a FETCH command to retrieve the specified message.
     * Can fetch headers only or the entire message based on options.
     *
     * @param id The unique identifier of the email message to download.
     * @return A string containing the email message. Returns an empty string on failure.
     */
    std::string downloadMessage(int id);

    /**
     * @brief Saves the given message content to a file with a specified ID.
     *
     * Saves the message content to a file in the output directory.
     *
     * @param message_content The content of the message to be saved.
     * @param id The identifier used to generate the filename.
     * @return int Returns 0 on success, or 1 on error.
     */
    int saveMessage(const std::string &message_content, int id);

    /**
     * @brief Prepares the output directory for use.
     *
     * Checks if the output directory exists. If it exists, removes all files within it.
     * If it does not exist, attempts to create the directory.
     *
     * @return int Returns 0 on success, or 1 on failure.
     */
    int prepareOutputDir();

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