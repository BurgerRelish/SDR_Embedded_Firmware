#pragma once

#ifndef PS_STRINGSTREAM_H
#define  PS_STRINGSTREAM_H

#include <sstream>
#include "ps_allocator.h"

namespace ps {
    using stringstream = std::basic_stringstream<char, std::char_traits<char>, allocator<char>>;
}

#endif