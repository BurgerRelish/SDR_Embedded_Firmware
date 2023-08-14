#pragma once

#ifndef COMMAND_SEPARATOR_H
#define COMMAND_SEPARATOR_H

#include "Semantics.h"
#include "Language.h"
#include "Lexer.h"
#include <ps_stl.h>

namespace re {
    
class CommandSeparator {
    private:
    
    public:
    CommandSeparator() {}

    ps::vector<std::tuple<ps::string, ps::vector<ps::string>>> separate(ps::string command);
};

}

#endif