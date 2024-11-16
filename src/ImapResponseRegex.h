#ifndef REGEX_PATTERNS_H
#define REGEX_PATTERNS_H

#include <regex>
#include <string>

// IP ADDRESS REGEX
const std::regex IP("^(\\d{1,3}\\.){3}\\d{1,3}$");

// GREETING
const std::regex GREETING_OK(R"(^\*\s+OK)");
const std::regex GREETING_PREAUTH(R"(^\*\s+PREAUTH)");
const std::regex GREETING_BYE(R"(^\*\s+BYE)");

// LOGIN
const std::regex LOGIN_OK(R"(^\s*A[0-9]+\s+OK)");
const std::regex LOGIN_NO(R"(^\s*A[0-9]+\s+NO\s*(.*))");
const std::regex LOGIN_BAD(R"(^\s*A[0-9]+\s+BAD\s*(.*))");

// SELECT
const std::regex SELECT_OK(R"(^\s*A[0-9]+\s+OK)");
const std::regex SELECT_NO(R"(^\s*A[0-9]+\s+NO\s*(.*))");
const std::regex SELECT_BAD(R"(^\s*A[0-9]+\s+BAD\s*(.*))");

// SEARCH
const std::regex SEARCH_RESPONSE_REGEX(R"(^\*\s+SEARCH\s+(.*)$)");
const std::regex SEARCH_OK(R"(^\s*A[0-9]+\s+OK)");

// FETCH
const std::regex FETCH_OK(R"(^\s*A[0-9]+\s+OK)");
const std::regex FETCH_NO(R"(^\s*A[0-9]+\s+NO\s*(.*))");
const std::regex FETCH_BAD(R"(^\s*A[0-9]+\s+BAD\s*(.*))");

const std::regex FETCH_MESSAGE(R"(\*\s+\d+\s+FETCH\s+\(.*?\{(\d+)\}\s+)");

// FOR TAG CHECKING
const std::regex RESPONSE_TAG_REGEX(R"((A[0-9]+)\s+(OK|BAD|NO|PREAUTH|BYE))");

#endif // REGEX_PATTERNS_H