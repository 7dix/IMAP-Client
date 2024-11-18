// AuthReader.cpp
// author: Marek Tenora
// login: xtenor02

#include "AuthReader.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

AuthReader::AuthReader(const std::string authFile) : authFile_(authFile) {}

AuthData AuthReader::read() {
    // Open the authentication file

    std::ifstream file(authFile_);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open authentication file.");
    }

    AuthData authData;
    std::string line;
    while (std::getline(file, line)) {
        if (authData.username != "" && authData.password != "") {
            break;
        }
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {

            // Remove leading and trailing spaces from key and value
            key.erase(0, key.find_first_not_of(' '));
            key.erase(key.find_last_not_of(' ') + 1);
            value.erase(0, value.find_first_not_of(' '));
            value.erase(value.find_last_not_of(' ') + 1);

            if (key == "username") {
                authData.username = value;
            } else if (key == "password") {
                authData.password = value;
            }
        } else {
            throw std::runtime_error("Failed to read authentication file.");
        }
    }

    return authData;
}