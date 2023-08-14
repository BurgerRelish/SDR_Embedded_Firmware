#pragma once

#ifndef RULE_H
#define RULE_H

#include "Function.h"
#include "Expression.h"

namespace re {

class Rule : private Expression, private Executor {
    public:
    int priority;

    Rule(const int rule_priority, const ps::string& expression_str, const ps::string& command_str, VariableStorage* variables, std::shared_ptr<FunctionStorage>& functions) : 
        priority(rule_priority), Expression(expression_str, variables), Executor(command_str, functions, variables)
    {}

    /**
     * @brief Evaluates the provided expression, if the expression evaluates to true, it executes the assosciated commands.
     * 
     * @return true - Evaluation and Execution were successful.
     * @return false - Either Evaluation or Execution failed.
     */
    bool reason() {
        if (Expression::evaluate()) {
            return Executor::execute();
        } else return false;
    }
    
};

}

#endif