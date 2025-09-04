#pragma once
#include <string>
#include <sstream>

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