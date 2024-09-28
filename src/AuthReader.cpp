// AuthReader.cpp
#include "AuthReader.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

AuthReader::AuthReader(const std::string authFile) : authFile_(authFile) {}

AuthData AuthReader::read() {
    std::ifstream file(authFile_);
    if (!file.is_open()) {
        throw std::runtime_error("Soubor s autentizačními údaji se nepodařilo otevřít");
    }

    AuthData authData;
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string username;
        std::string password;
        if (!(iss >> username >> password)) {
            throw std::runtime_error("Chyba při čtení autentizačního souboru");
        }
        authData = {username, password};
    }

    return authData;
}
