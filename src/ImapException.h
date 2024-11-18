// ImapException.h
// author: Marek Tenora
// login: xtenor02

#pragma once
#include <string>
#include <exception>

class ImapException : public std::runtime_error {
public:
    explicit ImapException(const std::string& message) : std::runtime_error(message) {}
};