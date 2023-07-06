#pragma once

#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "ArraySeparator.h"
#include "ShuntingYard.h"
#include "Lexer.h"
#include "sdr_containers.h"
#include "ps_queue.h"
#include "Language.h"

class Evaluator {
    private:
    struct LexedRule{
        int priority;
        ps_queue<Token> expression;
        ps_queue<Token> commands;
    };

    Module& module_class;
    SDRUnit& global_class;

    ps_queue<LexedRule> rules;

    void generateRules(ps_queue<Rule>& rule_input);
    ps_queue<Token> lexExpression(ps_string& expr);

    bool evaluateRPN(ps_queue<Token>& tokens);
    ps_queue<ps_string> separateArray(Token& array);

    public:
    Evaluator(SDRUnit& global_vars, Module& module_vars, ps_queue<Rule>& rule_input) :
    global_class(global_vars),
    module_class(module_vars)
    {
        generateRules(rule_input);
    }

    ps_queue<Token> evaluate();
    

};

#endif // EVALUATOR_H