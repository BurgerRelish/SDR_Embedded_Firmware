#pragma once

#ifndef EVALUATOR_H
#define EVALUATOR_H

#include <functional>

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

    ps_vector<LexedRule> rules;

    void generateRules(ps_queue<Rule>& rule_input);
    ps_queue<Token> lexExpression(ps_string& expr);

    bool evaluateRPN(ps_queue<Token>&);
    /* Operations */

    void evaluateOperator(ps_stack<Token>& tokens, Token& operator_token);
    bool applyBooleanOperator(Token& lhs, Token& rhs, Token& operator_token);
    double applyArithmeticOperator(Token& lhs, Token& rhs, Token& operator_token);
    bool applyComparisonOperator(Token& lhs, Token& rhs, Token& operator_token);
    bool applyArrayComparison(Token& lhs, Token& rhs, Token& operator_token);

    /* Variable Resolution */

    const VarType& getVarType(ps_string& search_str);

    const double getDouble(Token& token);
    const bool getBool(Token& token);
    const int getInt(Token& token);
    const uint64_t getUint64(Token& token);
    const ps_string getString(Token& token);

    void retrieveVar(const ps_string& var, double& val);
    void retrieveVar(const ps_string& var, bool& val);
    void retrieveVar(const ps_string& var, int& val);
    void retrieveVar(const ps_string& var, uint64_t& val);
    void retrieveVar(const ps_string& var, ps_string& val);

    double toDouble(ps_string& str);

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