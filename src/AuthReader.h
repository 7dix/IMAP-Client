// ArgumentsParser.h
#ifndef AUTHREADER_H
#define AUTHREADER_H

#include <string>
#include <vector>
#include <map>

/**
 * @brief Structure to hold authentication data.
 */
struct AuthData {
    std::string username; ///< Username for authentication.
    std::string password; ///< Password for authentication.

    bool operator==(const AuthData& other) const {
        return username == other.username && password == other.password;
    }
};

/**
 * @brief Class to read authentication data from a file.
 */
class AuthReader {
    public:
        /**
         * @brief Construct a new AuthReader object.
         * 
         * @param authFile Path to the authentication file.
         */
        AuthReader(std::string authFile);

        /**
         * @brief Read authentication data from the file.
         * 
         * @return AuthData Structure containing username and password.
         */
        AuthData read();

    private:
        std::string authFile_; ///< Path to the authentication file.
};

#endif // AUTHREADER_H