#pragma once

#include "Semantics.h"
#include "Language.h"
#include "VariableStorage.h"
#include "ArraySeparator.h"
#include "Lexer.h"
#include "ShuntingYard.h"

#include <ps_stl.h>

namespace re {

class Expression {
    private:
    ps::vector<Token> _expression;
    VariableStorage* variables;
    ps::unordered_map<ps::string, ps::vector<ps::string>> array_lookup;

    double evaluateRPN();
    
    /* Operations */
    void evaluateOperator(ps::stack<Token>& tokens, Token& operator_token);
    bool applyBooleanOperator(Token& lhs, Token& rhs, Token& operator_token);
    double applyArithmeticOperator(Token& lhs, Token& rhs, Token& operator_token);
    bool applyComparisonOperator(Token& lhs, Token& rhs, Token& operator_token);
    bool applyComparisonOperatorUint64(Token& lhs, Token& rhs, Token& operator_token);
    bool applyArrayComparison(Token& lhs, Token& rhs, Token& operator_token);
    bool applyStringComparison(Token& lhs, Token& rhs, Token& operator_token);
    Token applyArrayOperator(Token& lhs, Token& rhs, Token& operator_token);


    ps::vector<ps::string> separateArray(const Token& token);
    Token combineArray(ps::vector<ps::string>& array);
    const bool arrayMinQuantifierSearch(const ps::vector<ps::string>& lhs_array, const ps::vector<ps::string>& rhs_array, const size_t n) const;
    const bool arrayEqualityComparison(const ps::vector<ps::string>& lhs_array, const ps::vector<ps::string>& rhs_array) const;
    const bool arraySubsetComparison(const ps::vector<ps::string>& lhs_array, const ps::vector<ps::string>& rhs_array) const;

    public:
    Expression(const ps::string& expression, VariableStorage* vars) : variables(vars)
    {
        Lexer lexer;
        auto expr = lexer.tokenize(expression);
        expr = ShuntingYard::apply(expr);

        /* Load the expression into a vector. */
        while(!expr.empty()) {
            auto token = expr.front();
            _expression.push_back(token);
            expr.pop();
        }
    }

    Expression(ps::vector<Token>& expression, VariableStorage* vars) : variables(vars) {
        ps::queue<Token> to_eval;
        for (auto tok : expression) {
            to_eval.push(tok);
        }

        auto expr = ShuntingYard::apply(to_eval);

        /* Load the expression back into a vector. */
        while(!expr.empty()) {
            auto token = expr.front();
            _expression.push_back(token);
            expr.pop();
        }
    
    }

    bool evaluate();
    double result();

};

}
