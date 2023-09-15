
#ifndef PS_STRING_H
#define PS_STRING_H

#include <string>
#include "ps_allocator.h"

namespace ps {
    /**
     * @brief An implementation of std::string which is stored on the PSRAM. Member functions and their functionality is identical to that of std::string.
     * @note The operator '<<=' Can be used to copy the ps::string (PSRAM) to a std::string (SRAM) or vice-versa. 
    */
    using string = std::basic_string<char, std::char_traits<char>, allocator<char>>;

}

bool operator==(const ps::string& ps_str, const std::string& str);
bool operator==(const std::string& str, const ps::string& ps_str);
bool operator!=(const ps::string& ps_str, const std::string& str);
bool operator!=(const std::string& str, const ps::string& ps_str);
bool operator<(const ps::string& ps_str, const std::string& str);
bool operator<(const std::string& str, const ps::string& ps_str);
bool operator>(const ps::string& ps_str, const std::string& str);
bool operator>(const std::string& str, const ps::string& ps_str);
bool operator<=(const ps::string& ps_str, const std::string& str);
bool operator<=(const std::string& str, const ps::string& ps_str);
bool operator>=(const ps::string& ps_str, const std::string& str);
bool operator>=(const std::string& str, const ps::string& ps_str);
ps::string operator+=(ps::string& ps_str, const std::string& str);
std::string operator+=(std::string& str, const ps::string& ps_str);
ps::string operator+(ps::string& ps_str, const std::string& str);
std::string operator+(std::string& str, const ps::string& ps_str);
ps::string operator<<=(ps::string& ps_str, const std::string& str);
std::string operator<<=(std::string& str, const ps::string& ps_str);

#endif