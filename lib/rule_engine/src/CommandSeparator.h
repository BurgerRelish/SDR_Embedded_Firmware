#pragma once

#ifndef COMMAND_SEPARATOR_H
#define COMMAND_SEPARATOR_H

#include "../rule_engine/Semantics.h"
#include "Language.h"
#include "Lexer.h"
#include <ps_stl.h>

namespace re {
    
class CommandSeparator {
    private:
    
    public:
    CommandSeparator() {}

    static ps::vector<std::pair<ps::string, ps::vector<Token>>> separate(ps::string& command);
};

}

#endif