#pragma once

#ifndef SDR_LANGUAGE_H
#define SDR_LANGUAGE_H

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>

#include "../sdr_containers/SDRUnit.h"
#include "../sdr_containers/SDRModule.h"

#include "../ps_stl/ps_stl.h"

#include "Semantics.h"

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



enum OriginType {
    ORIG_UNIT,
    ORIG_MOD
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