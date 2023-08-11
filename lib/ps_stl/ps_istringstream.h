#pragma once

#ifndef PS_ISTRINGSTREAM_H
#define PS_ISTRINGSTREAM_H

#include <sstream>
#include "ps_allocator.h"

namespace ps {
    using istringstream = std::basic_istringstream<char, std::char_traits<char>, allocator<char>>;
}

#endif