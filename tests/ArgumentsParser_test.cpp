// tests/ArgumentsParser_test.cpp
#include <gtest/gtest.h>
#include "../src/ArgumentsParser.h"

class ArgumentsParserTest : public ::testing::Test {
protected:
    ArgumentsParser parser;
};

TEST_F(ArgumentsParserTest, ParsesMandatoryArguments) {
    char* argv[] = { (char*)"imapcl", (char*)"server_address", (char*)"-a", (char*)"auth_file", (char*)"-o", (char*)"output_dir" };
    int argc = 6;
    ProgramOptions options = parser.parse(argc, argv);

    EXPECT_EQ(options.server, "server_address");
    EXPECT_EQ(options.authFile, "auth_file");
    EXPECT_EQ(options.outputDir, "output_dir");
}

TEST_F(ArgumentsParserTest, ParsesOptionalArguments) {
    char* argv[] = { (char*)"imapcl", (char*)"server_address", (char*)"-a", (char*)"auth_file", (char*)"-o", (char*)"output_dir", (char*)"-p", (char*)"1234", (char*)"-T", (char*)"-c", (char*)"cert_file", (char*)"-C", (char*)"cert_dir", (char*)"-n", (char*)"-h", (char*)"-b", (char*)"mailbox" };
    int argc = 17;
    ProgramOptions options = parser.parse(argc, argv);

    EXPECT_EQ(options.server, "server_address");
    EXPECT_EQ(options.authFile, "auth_file");
    EXPECT_EQ(options.outputDir, "output_dir");
    EXPECT_EQ(options.port, 1234);
    EXPECT_TRUE(options.useTLS);
    EXPECT_EQ(options.certFile, "cert_file");
    EXPECT_EQ(options.certDir, "cert_dir");
    EXPECT_TRUE(options.onlyNewMessages);
    EXPECT_TRUE(options.headersOnly);
    EXPECT_EQ(options.mailbox, "mailbox");
}

TEST_F(ArgumentsParserTest, ThrowsOnMissingMandatoryArguments) {
    char* argv[] = { (char*)"imapcl", (char*)"server_address" };
    int argc = 2;
    EXPECT_THROW(parser.parse(argc, argv), std::invalid_argument);
}

TEST_F(ArgumentsParserTest, ThrowsOnInvalidPort) {
    char* argv[] = { (char*)"imapcl", (char*)"server_address", (char*)"-a", (char*)"auth_file", (char*)"-o", (char*)"output_dir", (char*)"-p", (char*)"70000" };
    int argc = 8;
    EXPECT_THROW(parser.parse(argc, argv), std::invalid_argument);
}

TEST_F(ArgumentsParserTest, SetsDefaultPortBasedOnTLS) {
    char* argv[] = { (char*)"imapcl", (char*)"server_address", (char*)"-a", (char*)"auth_file", (char*)"-o", (char*)"output_dir", (char*)"-T" };
    int argc = 7;
    ProgramOptions options = parser.parse(argc, argv);

    EXPECT_EQ(options.port, 993);
}

TEST_F(ArgumentsParserTest, SetsDefaultPortWithoutTLS) {
    char* argv[] = { (char*)"imapcl", (char*)"server_address", (char*)"-a", (char*)"auth_file", (char*)"-o", (char*)"output_dir" };
    int argc = 6;
    ProgramOptions options = parser.parse(argc, argv);

    EXPECT_EQ(options.port, 143);
}