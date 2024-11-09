#ifndef REGEX_PATTERNS_H
#define REGEX_PATTERNS_H

#include <regex>
#include <string>

// GREETING
const std::regex GREETING_OK(R"(^\*\s+OK)");
const std::regex GREETING_PREAUTH(R"(^\*\s+PREAUTH)");
const std::regex GREETING_BYE(R"(^\*\s+BYE)");

// LOGIN
const std::regex LOGIN_OK(R"(^a[0-9]{3}\s+OK)");
const std::regex LOGIN_NO(R"(^a[0-9]{3}\s+NO\s*(.*))");
const std::regex LOGIN_BAD(R"(^a[0-9]{3}\s+BAD\s*(.*))");

#endif // REGEX_PATTERNS_H
