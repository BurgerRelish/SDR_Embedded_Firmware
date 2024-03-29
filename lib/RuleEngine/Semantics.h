#pragma once

#ifndef SEMANTICS_H
#define SEMANTICS_H

/* Rule Engine Variables */
#define LAST_EXECUTION_TIME "last_tm"
#define CURRENT_TIME "tm"
#define INITIALIZED "setup" 

/* Rule Engine Functions */
#define SET_VAR "set"
#define MAKE_VAR "let"

/* Operators */
#define BOOLEAN_AND "&&"
#define BOOLEAN_OR "||"
#define BOOLEAN_NOT "!"

#define ARITHMETIC_ADD "+"
#define ARITHMETIC_SUBTRACT "-"
#define ARITHMETIC_MULTIPLY "*"
#define ARITHMETIC_DIVIDE "/"
#define ARITHMETIC_POWER "^"
#define ARITHMETIC_MODULUS "%"

#define COMPARISON_EQUAL "=="
#define COMPARISON_NOT_EQUAL "!="
#define COMPARISON_GREATER_THAN ">"
#define COMPARISON_LESSER_THAN "<"
#define COMPARISON_GREATER_THAN_OR_EQUAL ">="
#define COMPARISON_LESSER_THAN_OR_EQUAL "<="

#define ARRAY_TAG_EQUALITY_COMPARISON COMPARISON_EQUAL
#define ARRAY_TAG_INEQUALITY_COMPARISON COMPARISON_NOT_EQUAL
#define ARRAY_TAG_SUBSET_COMPARISON BOOLEAN_OR

#define ARRAY_SEPARATOR ","
#define COMMAND_SEPARATOR ";"
#define ARGUMENT_SEPARATOR ","
#define STRING_LITERAL_QUOTATION "\""

#endif