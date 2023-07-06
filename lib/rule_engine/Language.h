#pragma once

#ifndef SDR_LANGUAGE_H
#define SDR_LANGUAGE_H 1 

#define DEBUG_RULE_ENGINE

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>
#include "ps_string.h"
#include "ps_stack.h"
#include "ps_vector.h"

enum TokenType {
    LEFT_PARENTHESES, // (
    RIGHT_PARENTHESES, // )
    ARRAY, // ]
    ARITHMETIC_OPERATOR, // + - / ^ %
    BOOLEAN_OPERATOR, // && || !
    COMPARISON_OPERATOR, // == != <= >= < >
    STRING_LITERAL, // "Hello World!"
    NUMERIC_LITERAL, // 1 1.201
    IDENTIFIER,
    SEPARATOR // ,
};

struct Token {
    TokenType type;
    ps_string lexeme;
};

#endif // SDR_LANGUAGE_H