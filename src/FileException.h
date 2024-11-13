#pragma once
#include <string>
#include <stdexcept>

class FileException : public std::runtime_error {
public:
    explicit FileException(const std::string& message) : std::runtime_error(message) {}
};