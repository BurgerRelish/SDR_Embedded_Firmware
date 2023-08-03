#pragma once

#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <functional>

#include "ArraySeparator.h"
#include "ShuntingYard.h"
#include "Lexer.h"
#include "../sdr_containers/SDRUnit.h"
#include "../sdr_containers/SDRModule.h"
#include "ps_queue.h"
#include "Language.h"
#include "VariableLookup.h"

class Evaluator : private VariableLookup {
    private:
    struct LexedRule{
        int priority;
        ps_queue<Token> expression;
        ps_queue<Token> commands;
    };

    Module* module_class;
    SDRUnit* global_class;
    OriginType origin;

    ps_vector<LexedRule> rules;

    void generateRules(const ps_vector<Rule>& rule_input);
    ps_queue<Token> lexExpression(ps_string& expr);

    bool evaluateRPN(ps_queue<Token>&);
    /* Operations */

    void evaluateOperator(ps_stack<Token>& tokens, Token& operator_token);
    bool applyBooleanOperator(Token& lhs, Token& rhs, Token& operator_token);
    double applyArithmeticOperator(Token& lhs, Token& rhs, Token& operator_token);
    bool applyComparisonOperator(Token& lhs, Token& rhs, Token& operator_token);
    bool applyComparisonOperatorUint64(Token& lhs, Token& rhs, Token& operator_token);
    bool applyArrayComparison(Token& lhs, Token& rhs, Token& operator_token);
    bool applyStringComparison(Token& lhs, Token& rhs, Token& operator_token);

    public:
    Evaluator(SDRUnit* global_vars, Module* module_vars, OriginType _origin) :
    global_class(global_vars),
    module_class(module_vars),
    origin(_origin),
    VariableLookup(global_class, module_class)
    {
        if (origin == ORIG_MOD) {
            generateRules(module_class -> getRules());
        } else {
            generateRules(global_class -> getRules());
        }
    }

    Command evaluate();
};

#endif // EVALUATOR_H