#pragma once

#ifndef SHUNTINGYARD_H
#define SHUNTINGYARD_H

#include <stdexcept>

#include "ps_queue.h"
#include "ps_stack.h"
#include "ps_string.h"
#include "Language.h"

const std::unordered_map<TokenType, int> precedenceTable = {
    {LEFT_PARENTHESES, 0},
    {RIGHT_PARENTHESES, 0},
    {ARITHMETIC_OPERATOR, 1},
    {BOOLEAN_OPERATOR, 2},
    {COMPARISON_OPERATOR, 3}
};

class ShuntingYard {
public:
    /**
     * @brief Applies the shunting yard algorithm to the token queue.
     * 
     * This method takes a std::queue of tokens and applies the shunting yard algorithm
     * to convert the infix expression into a postfix expression.
     *
     * @param tokenQueue The input token queue.
     * @return The postfix expression as a std::queue of tokens.
     */
    static ps_queue<Token> apply(ps_queue<Token>& tokenQueue);
private:
    /**
     * @brief Checks if operator1 has higher precedence than operator2.
     *
     * @param operator1 The first operator token.
     * @param operator2 The second operator token.
     * @return True if operator1 has higher precedence, False otherwise.
     */
    static bool hasHigherPrecedence(const Token& operator1, const Token& operator2) {
        auto op1 = precedenceTable.find(operator1.type);
        auto op2 = precedenceTable.find(operator2.type);
        return op1 -> second > op2 -> second;
    }
};

#endif // SHUNTINGYARD_H