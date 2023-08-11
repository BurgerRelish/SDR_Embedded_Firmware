#pragma once

#ifndef RULE_SET_H
#define RULE_SET_H

#include "Lexer.h"
#include "VariableLookup.h"
#include "../ps_stl/ps_stl.h"

#include "../sdr_containers/SDRUnit.h"
#include "../sdr_containers/SDRModule.h"

class RuleSet {
    public:
    int priority;
    ps::queue<Token> expression;
    ps::queue<Token> command;

    RuleSet(int rule_priority, ps::string& expression_str, ps::string& command_str) : priority(rule_priority)
    {  
        {
            Lexer expr_lexer(expression_str);
            expression = expr_lexer.tokenize();
        }

        Lexer cmd_lexer(command_str);
        command = cmd_lexer.tokenize();
    }
};

#endif