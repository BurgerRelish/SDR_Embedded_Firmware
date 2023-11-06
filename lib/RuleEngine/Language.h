#pragma once

#ifndef SDR_LANGUAGE_H
#define SDR_LANGUAGE_H

#include <ps_stl.h>

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
    ps::string lexeme;
};

enum VarType{
    DOUBLE,
    BOOL,
    INT,
    UINT64_T,
    PS_STRING,
    ARRAY_VAR,
    INVALID_VAR
};


#endif // SDR_LANGUAGE_H