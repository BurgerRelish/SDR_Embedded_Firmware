#pragma once

#include "Semantics.h"
#include "Language.h"
#include "VariableStorage.h"
#include "ArraySeparator.h"
#include "Lexer.h"
#include "ShuntingYard.h"

#include "../ps_stl/ps_stl.h"

namespace re {

class Expression {
    private:
    ps::queue<Token> _expression;
    std::shared_ptr<VariableStorage> variables;

    bool evaluateRPN();
    /* Operations */

    void evaluateOperator(ps::stack<Token>& tokens, Token& operator_token);
    bool applyBooleanOperator(Token& lhs, Token& rhs, Token& operator_token);
    double applyArithmeticOperator(Token& lhs, Token& rhs, Token& operator_token);
    bool applyComparisonOperator(Token& lhs, Token& rhs, Token& operator_token);
    bool applyComparisonOperatorUint64(Token& lhs, Token& rhs, Token& operator_token);
    bool applyArrayComparison(Token& lhs, Token& rhs, Token& operator_token);
    bool applyStringComparison(Token& lhs, Token& rhs, Token& operator_token);

    public:
    Expression(const ps::string& expression, std::shared_ptr<VariableStorage> vars) : variables(vars)
    {
        Lexer lexer(expression);
        _expression = lexer.tokenize();
        _expression = ShuntingYard::apply(_expression);
    }

    bool evaluate();

};

}
