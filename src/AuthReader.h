// ArgumentsParser.h
#ifndef AUTHREADER_H
#define AUTHREADER_H

#include <string>
#include <vector>
#include <map>

struct AuthData {
    std::string username;
    std::string password;
};

class AuthReader {
    public:
        AuthReader(std::string authFile);
        AuthData read();
    private:
        std::string authFile_;
};

#endif // AUTHREADER_H