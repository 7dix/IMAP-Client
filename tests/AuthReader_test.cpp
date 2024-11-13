#include "gtest/gtest.h"
#include "../src/AuthReader.h"
#include <fstream>

class AuthReaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create a temporary auth file for testing
        std::ofstream outFile("test_auth.txt");
        outFile << "username=testuser\n";
        outFile << "password=testpass\n";
        outFile.close();
    }

    void TearDown() override {
        // Remove the temporary auth file after testing
        std::remove("test_auth.txt");
    }
};

TEST_F(AuthReaderTest, ReadValidAuthFile) {
    AuthReader reader("test_auth.txt");
    AuthData data = reader.read();
    EXPECT_EQ(data.username, "testuser");
    EXPECT_EQ(data.password, "testpass");
}

TEST_F(AuthReaderTest, FileNotFound) {
    AuthReader reader("nonexistent.txt");
    EXPECT_THROW(reader.read(), std::runtime_error);
}

TEST_F(AuthReaderTest, InvalidFormat) {
    std::ofstream outFile("test_auth_invalid.txt");
    outFile << "username=testuser\n";
    outFile << "invalid_line_without_equals_sign\n";
    outFile.close();

    AuthReader reader("test_auth_invalid.txt");
    EXPECT_THROW(reader.read(), std::runtime_error);

    std::remove("test_auth_invalid.txt");
}