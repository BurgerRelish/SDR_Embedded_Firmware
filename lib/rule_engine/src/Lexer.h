#pragma once

#ifndef LEXER_H
#define LEXER_H

#include "Language.h"
#include <ps_stl.h>


/**
 * @class Lexer
 * @brief Tokenizes standard C expressions.
 */
class Lexer {
private:
    ps::string expression_;
    size_t index;

public:
    Lexer(const std::string& expression) : index(0) {
        expression_ <<= expression; // Move the std::string to PSRAM for processing.
    }

    Lexer(const ps::string& expression) : expression_(expression), index(0) {}
    Lexer() {};
    /**
     * @brief Tokenize the input expression and return a queue of tokens.
     * @param expression The C expression to tokenize.
     * @returns std::queue<std::string> A queue of tokens.
     */
    ps::queue<Token> tokenize();

    ps::queue<Token> tokenize(const ps::string& expression) {
        index = 0;
        expression_ = expression;
        return tokenize();
    }

private:
    /**
     * @brief Check if the character is whitespace.
     * @param ch The character to check.
     * @returns bool True if the character is whitespace, false otherwise.
     */
    bool isWhitespace(const char ch) const;

    /**
     * @brief Check if the character is an arithmetic operator.
     * @param ch The character to check.
     * @returns bool True if the character is an operator, false otherwise.
     */
    bool isArithmeticOperator(const char ch) const;

    /**
     * @brief Check if the character is an boolean operator.
     * @param ch The character to check.
     * @returns bool True if the character is an operator, false otherwise.
     */
    bool isBooleanOperator(const char ch) const;

    /**
     * @brief Check if the character is a string literal.
     * @param ch The character to check.
     * @returns bool True if the character is an operator, false otherwise.
     */
    bool isStringLiteral(const char ch) const;

    /**
     * @brief Check if the character is a numeric literal.
     * @param ch The character to check.
     * @returns bool True if the character is an operator, false otherwise.
     */
    bool isNumericLiteral(const char ch) const;

    /**
     * @brief Check if the character is a parenthesis.
     * @param ch The character to check.
     * @returns bool True if the character is a parenthesis, false otherwise.
     */
    bool isParenthesis(const char ch) const;

    bool isComparisonOperator(const char ch) const;
    bool isArray(const char ch) const;
    bool isSeparator(const char ch) const;

    /**
     * @brief Convert a boolean operator to a token.
     * @returns Token The processed token.
     */
    Token handleBooleanOperator();

    /**
     * @brief Convert a arithmetic operator to a token.
     * @returns Token The processed token.
     */
    Token handleArithmeticOperator();

    /**
     * @brief Convert a string literal to a token.
     * @returns Token The processed token.
     */
    Token handleStringLiteral();

    /**
     * @brief Convert a numeric literal to a token.
     * @returns Token The processed token.
     */
    Token handleNumericLiteral();

    /**
     * @brief Convert an identifier to a token.
     * @returns Token The processed token.
     */
    Token handleIdentifier();

    /**
     * @brief Convert a parenthesis to a token.
     * @returns Token The processed token.
     */
    Token handleParenthesis();

    Token handleArray();
    Token handleComparisonOperator();
    Token handleSeparator();

};

#endif  // LEXER_H