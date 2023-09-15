#pragma once

#ifndef PS_OSTRINGSTREAM_H
#define PS_OSTRINGSTREAM_H

#include <sstream>
#include "ps_allocator.h"

namespace ps {
    using ostringstream = std::basic_ostringstream<char, std::char_traits<char>, allocator<char>>;
}

#endif