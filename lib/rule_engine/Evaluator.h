#pragma once

#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <functional>
#include <memory>

#include "ArraySeparator.h"
#include "ShuntingYard.h"
#include "Lexer.h"
#include "../sdr_containers/SDRUnit.h"
#include "../sdr_containers/SDRModule.h"
#include "../ps_stl/ps_stl.h"

#include "Language.h"
#include "VariableLookup.h"

struct Command;

class Evaluator : private VariableLookup, public std::enable_shared_from_this<Evaluator>  {
    private:
    struct LexedRule{
        int priority;
        ps::queue<Token> expression;
        ps::queue<Token> commands;
    };

    OriginType origin;

    uint64_t delay_end_time = 0;

    ps::vector<LexedRule> rules;

    void generateRules(const ps::vector<Rule>& rule_input);
    ps::queue<Token> lexExpression(ps::string& expr);

    bool evaluateRPN(ps::queue<Token>&);
    /* Operations */

    void evaluateOperator(ps::stack<Token>& tokens, Token& operator_token);
    bool applyBooleanOperator(Token& lhs, Token& rhs, Token& operator_token);
    double applyArithmeticOperator(Token& lhs, Token& rhs, Token& operator_token);
    bool applyComparisonOperator(Token& lhs, Token& rhs, Token& operator_token);
    bool applyComparisonOperatorUint64(Token& lhs, Token& rhs, Token& operator_token);
    bool applyArrayComparison(Token& lhs, Token& rhs, Token& operator_token);
    bool applyStringComparison(Token& lhs, Token& rhs, Token& operator_token);

    public:
    Evaluator(std::shared_ptr<SDRUnit> global_vars, std::shared_ptr<Module> module_vars, OriginType _origin) :
    origin(_origin),
    VariableLookup(global_vars, module_vars)
    {
        if (origin == ORIG_MOD) {
            generateRules(module->getRules());
        } else {
            generateRules(unit->getRules());
        }

        
    }

    Command evaluate();

    void setDelay(uint32_t delay_s) {
        delay_end_time = esp_timer_get_time() + (((uint64_t)delay_s) * 1000000);
    }
};


struct Command {
    int priority;
    OriginType type;
    std::shared_ptr<Evaluator> origin;
    ps::queue<Token> command;
};

#endif // EVALUATOR_H