
#ifndef PS_STRING_H
#define PS_STRING_H

#include <string>
#include "psram_allocator.h"

/**
 * @brief An implementation of std::string which is stored on the PSRAM. Member functions and their functionality is identical to that of std::string.
 * @note The operator '<<=' Can be used to copy the ps_string (PSRAM) to a std::string (SRAM) or vice-versa. 
*/
using ps_string = std::basic_string<char, std::char_traits<char>, PSRAMAllocator<char>>;

bool operator==(const ps_string& ps_str, const std::string& str);
bool operator==(const std::string& str, const ps_string& ps_str);
bool operator!=(const ps_string& ps_str, const std::string& str);
bool operator!=(const std::string& str, const ps_string& ps_str);
bool operator<(const ps_string& ps_str, const std::string& str);
bool operator<(const std::string& str, const ps_string& ps_str);
bool operator>(const ps_string& ps_str, const std::string& str);
bool operator>(const std::string& str, const ps_string& ps_str);
bool operator<=(const ps_string& ps_str, const std::string& str);
bool operator<=(const std::string& str, const ps_string& ps_str);
bool operator>=(const ps_string& ps_str, const std::string& str);
bool operator>=(const std::string& str, const ps_string& ps_str);
ps_string operator+=(ps_string& ps_str, const std::string& str);
std::string operator+=(std::string& str, const ps_string& ps_str);
ps_string operator+(ps_string& ps_str, const std::string& str);
std::string operator+(std::string& str, const ps_string& ps_str);
ps_string operator<<=(ps_string& ps_str, const std::string& str);
std::string operator<<=(std::string& str, const ps_string& ps_str);

#endif