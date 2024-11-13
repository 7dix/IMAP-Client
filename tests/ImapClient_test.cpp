// ImapClient_test.cpp
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "../src/ImapClient.h"
#include "../src/ImapParser.h"
#include "../src/ArgumentsParser.h"
#include "../src/ImapException.h"
#include <string>
#include <vector>

using ::testing::Return;
using ::testing::_;
using ::testing::Throw;

// Mock class inheriting from ImapClient
class MockImapClient : public ImapClient {
public:
    MockImapClient(ProgramOptions &options) : ImapClient(options) {}

    MOCK_METHOD(int, establishConnection, (), (override));
    MOCK_METHOD(int, TLSHandshake, (), (override));
    MOCK_METHOD(void, receiveGreeting, (), (override));
    MOCK_METHOD(void, login, (AuthData auth), (override));
    MOCK_METHOD(void, selectMailbox, (), (override));
    MOCK_METHOD(void, fetchMessages, (), (override));
    MOCK_METHOD(void, disconnect, (), (override));
    MOCK_METHOD(int, sendCommand, (const std::string &command), (override));
    MOCK_METHOD(std::string, receiveResponse, (), (override));
    MOCK_METHOD(std::string, downloadMessage, (int id), (override));
    MOCK_METHOD(std::string, recvData, (), (override));
};

// Test case for successful connection and login
TEST(ImapClientTest, SuccessfulConnectionAndLogin) {
    ProgramOptions options;
    options.server = "imap.example.com";
    options.port = 993;
    options.useTLS = true;

    AuthData auth;
    auth.username = "user@example.com";
    auth.password = "password";

    MockImapClient mockClient(options);

    EXPECT_CALL(mockClient, establishConnection())
        .Times(1)
        .WillOnce(Return(0));
    EXPECT_CALL(mockClient, TLSHandshake())
        .Times(1)
        .WillOnce(Return(0));
    EXPECT_CALL(mockClient, receiveGreeting())
        .Times(1);
    EXPECT_CALL(mockClient, login(auth))
        .Times(1);
    EXPECT_CALL(mockClient, selectMailbox())
        .Times(1);
    EXPECT_CALL(mockClient, fetchMessages())
        .Times(1);
    EXPECT_CALL(mockClient, disconnect())
        .WillRepeatedly(Return());

    // Simulate the client's behavior
    mockClient.connectImap();
    mockClient.receiveGreeting();
    mockClient.login(auth);
    mockClient.selectMailbox();
    mockClient.fetchMessages();
    mockClient.disconnect();
}

// Test case for failed login
TEST(ImapClientTest, FailedLogin) {
    ProgramOptions options;
    options.server = "imap.example.com";
    options.port = 993;
    options.useTLS = true;

    AuthData auth;
    auth.username = "user@example.com";
    auth.password = "wrongpassword";

    MockImapClient mockClient(options);

    EXPECT_CALL(mockClient, establishConnection())
        .Times(1)
        .WillOnce(Return(0));
    EXPECT_CALL(mockClient, TLSHandshake())
        .Times(1)
        .WillOnce(Return(0));
    EXPECT_CALL(mockClient, receiveGreeting())
        .Times(1);
    EXPECT_CALL(mockClient, login(auth))
        .Times(1)
        .WillOnce(Throw(ImapException("Login failed")));

    EXPECT_CALL(mockClient, disconnect())
        .Times(1);

    // Simulate the client's behavior
    mockClient.connectImap();
    mockClient.receiveGreeting();
    EXPECT_THROW(mockClient.login(auth), ImapException);
    mockClient.disconnect();
}

// Test case for server disconnection
TEST(ImapClientTest, ServerDisconnection) {
    ProgramOptions options;
    options.server = "imap.example.com";
    options.port = 993;
    options.useTLS = true;

    AuthData auth;
    auth.username = "user@example.com";
    auth.password = "password";

    MockImapClient mockClient(options);

    EXPECT_CALL(mockClient, establishConnection())
        .Times(1)
        .WillOnce(Return(0));
    EXPECT_CALL(mockClient, TLSHandshake())
        .Times(1)
        .WillOnce(Return(0));
    EXPECT_CALL(mockClient, receiveGreeting())
        .Times(1)
        .WillOnce(Throw(ImapException("Server disconnected")));

    EXPECT_CALL(mockClient, disconnect())
        .Times(1);

    // Simulate the client's behavior
    mockClient.connectImap();
    EXPECT_THROW(mockClient.receiveGreeting(), ImapException);
    mockClient.disconnect();
}

// Add new test case for message download
TEST(ImapClientTest, SuccessfulMessageDownload) {
    ProgramOptions options;
    options.server = "imap.example.com";
    options.port = 993;
    options.useTLS = true;
    options.mailbox = "INBOX";

    MockImapClient mockClient(options);

    // Setup expected behavior for message download
    EXPECT_CALL(mockClient, downloadMessage(1))
        .WillOnce(Return("From: test@example.com\r\n\r\nTest message"));

    std::string message = mockClient.downloadMessage(1);
    EXPECT_FALSE(message.empty());
    EXPECT_TRUE(message.find("From:") != std::string::npos);
}

// Test case for error handling during message fetch
TEST(ImapClientTest, MessageFetchError) {
    ProgramOptions options;
    options.server = "imap.example.com";
    options.port = 993;
    options.useTLS = true;

    MockImapClient mockClient(options);

    EXPECT_CALL(mockClient, downloadMessage(1))
        .WillOnce(Throw(ImapException("Failed to fetch message")));

    EXPECT_THROW(mockClient.downloadMessage(1), ImapException);
}