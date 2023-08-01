#pragma once

#ifndef SDR_LANGUAGE_H
#define SDR_LANGUAGE_H 1 

#include <string>
#include <vector>
#include <queue>
#include <unordered_map>

#include "../data_containers/ps_string.h"
#include "../data_containers/ps_stack.h"
#include "../data_containers/ps_vector.h"

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
    ps_string lexeme;
};

struct Command {
    int priority;
    ps_stack<Token> command;
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


/* SDR Unit Identifer Names */
const std::unordered_map<std::string, VarType> vartype_lookup {
    /* Global Variables */
    {TOTAL_ACTIVE_POWER, DOUBLE},
    {TOTAL_REACTIVE_POWER, DOUBLE},
    {TOTAL_APPARENT_POWER, DOUBLE},
    {POWER_STATUS, BOOL},
    {UNIT_ID, PS_STRING},
    {MODULE_COUNT, INT},
    {UNIT_TAG_LIST, ARRAY_VAR},
    {CURRENT_TIME, UINT64_T},

    /* Module Variables */
    {ACTIVE_POWER, DOUBLE},
    {REACTIVE_POWER, DOUBLE},
    {APPARENT_POWER, DOUBLE},
    {VOLTAGE, DOUBLE},
    {FREQUENCY, DOUBLE},
    {POWER_FACTOR, DOUBLE},
    {SWITCH_TIME, BOOL},
    {CIRCUIT_PRIORITY, INT},
    {MODULE_ID, PS_STRING},
    {MODULE_TAG_LIST, ARRAY_VAR},
    {SWITCH_TIME, UINT64_T},
    {SWITCH_STATUS, BOOL},
    {"INVALID_VAR", INVALID_VAR}
};


#endif // SDR_LANGUAGE_H