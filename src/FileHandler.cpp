// FileHandler.cpp

#include "FileHandler.h"
#include "FileException.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <system_error>

FileHandler::FileHandler(std::string mailFolder) : path(mailFolder) {}

void FileHandler::createDirectories(const std::string& fullPath) {
    std::error_code ec;
    if (!std::filesystem::create_directories(fullPath, ec)) {
        if (ec) {
            throw FileException("Nepodařilo se vytvořit složky pro cestu: " + fullPath + " - " + ec.message());
        }
    }
}

int FileHandler::isMessageAlreadyDownloaded(int id, std::string &account, std::string &mailbox) {
    try {
        std::string dirPath = path + "/" + account + "/" + mailbox;
        std::string filename = dirPath + "/" + std::to_string(id) + ".eml";

        // Check if file exists
        if (!std::filesystem::exists(filename)) {
            return 0; // Not downloaded
        }

        // Check if file is empty
        if (std::filesystem::file_size(filename) == 0) {
            return 0;
        }

        // Read file content
        std::ifstream file(filename);
        std::string line;
        bool foundBlankLine = false;
        bool hasContent = false;
        
        while (std::getline(file, line)) {
            if (line.empty() || line == "\r") {
                foundBlankLine = true;
                continue;
            }
            if (foundBlankLine && !line.empty()) {
                hasContent = true;
                break;
            }
        }

        return hasContent ? 1 : 2; // 1 = full message, 2 = headers only
    }
    catch (const std::filesystem::filesystem_error&) {
        return -1;
    }
}

void FileHandler::saveMessage(const std::string &message_content, int id, std::string &account, std::string &mailbox) {
    std::string dirPath = path + "/" + account + "/" + mailbox;
    std::string filename = dirPath + "/" + std::to_string(id) + ".eml";

    // Create directory structure if it doesn't exist
    createDirectories(dirPath);
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw FileException("Nepodařilo se otevřít soubor zprávy: " + filename);
    }
    file << message_content;
    file.close();
}
