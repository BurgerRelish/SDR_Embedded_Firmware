#pragma once

#ifndef COMMAND_SEPARATOR_H
#define COMMAND_SEPARATOR_H

#include "Semantics.h"
#include "Language.h"
#include "Lexer.h"
#include <ps_stl.h>

namespace re {
    
using CommandData = std::tuple<ps::string, ps::vector<ps::vector<ps::string>>>; // <command name, <arguments>>
class CommandSeparator {
    private:
    
    public:
    CommandSeparator() {}

    ps::vector<CommandData> separate(ps::string command);
};

}

#endif