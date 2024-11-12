#ifndef REGEX_PATTERNS_H
#define REGEX_PATTERNS_H

#include <regex>
#include <string>

// GREETING
const std::regex GREETING_OK(R"(^\*\s+OK)");
const std::regex GREETING_PREAUTH(R"(^\*\s+PREAUTH)");
const std::regex GREETING_BYE(R"(^\*\s+BYE)");

// LOGIN
const std::regex LOGIN_OK(R"(^A[0-9]+\s+OK)");
const std::regex LOGIN_NO(R"(^A[0-9]+\s+NO\s*(.*))");
const std::regex LOGIN_BAD(R"(^A[0-9]+\s+BAD\s*(.*))");

// SELECT
const std::regex SELECT_OK(R"(^A[0-9]+\s+OK)");
const std::regex SELECT_NO(R"(^A[0-9]+\s+NO\s*(.*))");
const std::regex SELECT_BAD(R"(^A[0-9]+\s+BAD\s*(.*))");

// SEARCH
const std::regex SEARCH_RESPONSE_REGEX(R"(^\*\s+SEARCH\s+(.*)$)");
const std::regex SEARCH_OK(R"(^A[0-9]+\s+OK)");

// FETCH
const std::regex FETCH_OK(R"(^A[0-9]+\s+OK)");
const std::regex FETCH_NO(R"(^A[0-9]+\s+NO\s*(.*))");
const std::regex FETCH_BAD(R"(^A[0-9]+\s+BAD\s*(.*))");

const std::regex FETCH_MESSAGE(R"(\* \d+ FETCH .*?\{(\d+)\}\r?\n)");

// FOR TAG CHECKING
const std::regex RESPONSE_TAG_REGEX(R"((A[0-9]+)\s+(OK|BAD|NO|PREAUTH|BYE))");

#endif // REGEX_PATTERNS_H