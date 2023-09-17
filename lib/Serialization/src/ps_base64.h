//
//  base64 encoding and decoding with C++.
//  Version: 2.rc.09 (release candidate)
//

#ifndef PS_BASE64_H_C0CE2A47_D10E_42C9_A27C_C883944E704A
#define PS_BASE64_H_C0CE2A47_D10E_42C9_A27C_C883944E704A

#include <string>
#include <ps_stl.h>


#if __cplusplus >= 201703L
#include <string_view>
#endif  // __cplusplus >= 201703L

ps::string base64_encode     (ps::string const& s, bool url = false);
ps::string base64_encode_pem (ps::string const& s);
ps::string base64_encode_mime(ps::string const& s);

ps::string base64_decode(ps::string const& s, bool remove_linebreaks = false);
ps::string base64_encode(unsigned char const*, size_t len, bool url = false);

#if __cplusplus >= 201703L
//
// Interface with std::string_view rather than const std::string&
// Requires C++17
// Provided by Yannic Bonenberger (https://github.com/Yannic)
//
std::string base64_encode     (std::string_view s, bool url = false);
std::string base64_encode_pem (std::string_view s);
std::string base64_encode_mime(std::string_view s);

std::string base64_decode(std::string_view s, bool remove_linebreaks = false);
#endif  // __cplusplus >= 201703L

#endif /* BASE64_H_C0CE2A47_D10E_42C9_A27C_C883944E704A */