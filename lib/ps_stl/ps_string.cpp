#include "ps_string.h"

bool operator==(const ps::string& ps_str, const std::string& str) {
    return ps_str == str.c_str();
}

bool operator==(const std::string& str, const ps::string& ps_str) {
    return str == ps_str.c_str();
}

bool operator!=(const ps::string& ps_str, const std::string& str) {
    return ps_str != str.c_str();
}

bool operator!=(const std::string& str, const ps::string& ps_str) {
    return str != ps_str.c_str();
}

bool operator<(const ps::string& ps_str, const std::string& str) {
    return ps_str < str.c_str();
}

bool operator<(const std::string& str, const ps::string& ps_str) {
    return str < ps_str.c_str();
}

bool operator>(const ps::string& ps_str, const std::string& str) {
    return ps_str > str.c_str();
}

bool operator>(const std::string& str, const ps::string& ps_str) {
    return str > ps_str.c_str();
}

bool operator<=(const ps::string& ps_str, const std::string& str) {
    return ps_str <= str.c_str();
}

bool operator<=(const std::string& str, const ps::string& ps_str) {
    return str <= ps_str.c_str();
}

bool operator>=(const ps::string& ps_str, const std::string& str) {
    return ps_str >= str.c_str();
}

bool operator>=(const std::string& str, const ps::string& ps_str) {
    return str >= ps_str.c_str();
}

ps::string operator+=(ps::string& ps_str, const std::string& str) {
    ps_str += str.c_str();
    return ps_str;
}

std::string operator+=(std::string& str, const ps::string& ps_str) {
    str += ps_str.c_str();
    return str;
}

ps::string operator+(ps::string& ps_str, const std::string& str) {
    return ps_str + ps::string(str.c_str());
}

std::string operator+(std::string& str, const ps::string& ps_str) {
    return str + std::string(ps_str.c_str());
}

ps::string operator<<=(ps::string& ps_str, const std::string& str) {
    ps_str = str.c_str();
    return ps_str;
}

std::string operator<<=(std::string& str, const ps::string& ps_str) {
    str = ps_str.c_str();
    return str;
}

