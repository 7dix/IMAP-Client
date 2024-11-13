#include <gtest/gtest.h>
#include "../src/ImapParser.h"
#include "../src/ImapException.h"

class ImapParserTest : public ::testing::Test {
protected:
    ImapParser parser;
};

TEST_F(ImapParserTest, ParseLoginResponseSuccess) {
    std::string response = "* OK [CAPABILITY IMAP4rev1] Logged in\r\n A1 OK \r\n";
    EXPECT_NO_THROW(parser.parseLoginResponse(response));
}

TEST_F(ImapParserTest, ParseLoginResponseFailureNo) {
    std::string response = "* NO [AUTH] Authentication failed\r\n A1 NO \r\n";
    EXPECT_THROW(parser.parseLoginResponse(response), ImapException);
}

TEST_F(ImapParserTest, ParseLoginResponseFailureBad) {
    std::string response = "* BAD [AUTH] Authentication failed\r\n A1 BAD \r\n";
    EXPECT_THROW(parser.parseLoginResponse(response), ImapException);
}

TEST_F(ImapParserTest, ParseSelectResponseSuccess) {
    std::string response = "* OK [READ-WRITE] Select completed\r\n A1 OK \r\n";
    EXPECT_NO_THROW(parser.parseSelectResponse(response));
}

TEST_F(ImapParserTest, ParseSelectResponseFailureNo) {
    std::string response = "* NO [NONEXISTENT] Mailbox does not exist\r\n A1 NO \r\n";
    EXPECT_THROW(parser.parseSelectResponse(response), ImapException);
}

TEST_F(ImapParserTest, ParseSelectResponseFailureBad) {
    std::string response = "* BAD [CLIENTBUG] Invalid command\r\n A1 BAD \r\n";
    EXPECT_THROW(parser.parseSelectResponse(response), ImapException);
}

TEST_F(ImapParserTest, ParseSearchResponse) {
    std::string response = "* SEARCH 1 2 3 4 5\r\n A1 OK \r\n";
    std::vector<int> expected = {1, 2, 3, 4, 5};
    EXPECT_EQ(parser.parseSearchResponse(response), expected);
}

TEST_F(ImapParserTest, ParseFetchResponseSuccess) {
    std::string response = "* 1 FETCH (BODY[] {12}\r\nHello World!)\r\n A1 OK \r\n";
    std::string expected = "Hello World!";
    EXPECT_EQ(parser.parseFetchResponse(response), expected);
}

TEST_F(ImapParserTest, ParseFetchResponseFailureNo) {
    std::string response = "* NO [TRYCREATE] Mailbox does not exist\r\n A1 NO \r\n";
    EXPECT_THROW(parser.parseFetchResponse(response), ImapException);
}

TEST_F(ImapParserTest, ParseFetchResponseFailureBad) {
    std::string response = "* BAD [CLIENTBUG] Invalid command\r\n A1 BAD \r\n";
    EXPECT_THROW(parser.parseFetchResponse(response), ImapException);
}

TEST_F(ImapParserTest, CheckResponseReceived) {
    std::string response = "A1 OK Completed\r\n ";
    std::string tag = "A1";
    EXPECT_TRUE(parser.checkResponseReceived(response, tag));
}

TEST_F(ImapParserTest, CheckResponseNotReceived) {
    std::string response = "A1 OK Completed\r\n";
    std::string tag = "A2";
    EXPECT_FALSE(parser.checkResponseReceived(response, tag));
}

TEST_F(ImapParserTest, ParseLoginResponseEmpty) {
    std::string response = "";
    EXPECT_THROW(parser.parseLoginResponse(response), ImapException);
}

TEST_F(ImapParserTest, ParseSelectResponseEmpty) {
    std::string response = "";
    EXPECT_THROW(parser.parseSelectResponse(response), ImapException);
}

TEST_F(ImapParserTest, ParseSearchResponseEmpty) {
    std::string response = "";
    std::vector<int> expected = {};
    EXPECT_EQ(parser.parseSearchResponse(response), expected);
}

TEST_F(ImapParserTest, ParseFetchResponseEmpty) {
    std::string response = "";
    EXPECT_THROW(parser.parseFetchResponse(response), ImapException);
}

TEST_F(ImapParserTest, CheckResponseReceivedEmpty) {
    std::string response = "";
    std::string tag = "A1";
    EXPECT_FALSE(parser.checkResponseReceived(response, tag));
}

TEST_F(ImapParserTest, ParseFetchResponsePartial) {
    std::string response = "* 1 FETCH (BODY[] {12}\r\nHello";
    EXPECT_THROW(parser.parseFetchResponse(response), ImapException);
}

TEST_F(ImapParserTest, ParseFetchResponseInvalidFormat) {
    std::string response = "* 1 FETCH BODY[] {12}\r\nHello World!\r\n A1 OK \r\n";
    EXPECT_THROW(parser.parseFetchResponse(response), ImapException);
}