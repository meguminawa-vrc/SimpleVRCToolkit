#pragma once
#include <string>
#include <sstream>
#include <unordered_map>

template<typename T>
std::string toString(const T& value) {
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

template<typename... Args>
std::string concatToString(const Args&... args) {
    std::ostringstream oss;
    (oss << ... << args); // fold expression (C++17)
    return oss.str();
}

std::string formatOSCMessage(
    const std::string& templateStr, 
    const std::unordered_map<std::string, std::string>& params
    );

void run_hidden(const wchar_t* exePath);