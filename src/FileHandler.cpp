// FileHandler.cpp
// author: Marek Tenora
// login: xtenor02

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
            throw FileException("Failed to create directories for path: " + fullPath + " - " + ec.message());
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
        throw FileException("Failed to open message file: " + filename);
    }
    file << message_content;
    file.close();
}

int FileHandler::checkMailboxUIDValidity(std::string &account, std::string &mailbox, int uidValidity) {
    std::string dirPath = path + "/" + account + "/" + mailbox;
    std::string uidValidityFile = dirPath + "/uidvalidity.txt";

    // Check if UIDVALIDITY file exists
    if (!std::filesystem::exists(uidValidityFile)) {
        // Create directory structure if it doesn't exist
        if (!std::filesystem::exists(dirPath)) {
            createDirectories(dirPath);
        } else {
            // UIDVALIDITY was missing but files were here.
            // Delete all .eml files from the directory
            for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
                if (entry.path().extension() == ".eml") {
                    std::filesystem::remove(entry.path());
                }
            }
        }

        // Write the new UIDVALIDITY value to the file
        std::ofstream file(uidValidityFile);
        if (!file.is_open()) {
            return -1; // Error creating file
        }
        file << uidValidity;
        file.close();
        return 2; // Mailbox did not exist, new UIDVALIDITY file created
    }

    // Read UIDVALIDITY value from file
    std::ifstream file(uidValidityFile);
    if (!file.is_open()) {
        return -1; // Error reading file
    }

    int storedUIDValidity;
    file >> storedUIDValidity;
    file.close();

    if (storedUIDValidity != uidValidity) {
        // Update the UIDVALIDITY value in the file
        std::ofstream outFile(uidValidityFile);
        if (!outFile.is_open()) {
            return -1; // Error updating file
        }
        outFile << uidValidity;
        outFile.close();

        // Delete all .eml files from the directory
        for (const auto& entry : std::filesystem::directory_iterator(dirPath)) {
            if (entry.path().extension() == ".eml") {
                std::filesystem::remove(entry.path());
            }
        }

        return 1; // UIDVALIDITY updated
    }

    return 0; // UIDVALIDITY matches
}
