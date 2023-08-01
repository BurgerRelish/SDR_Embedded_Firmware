#pragma once

#ifndef SEMANTICS_H
#define SEMANTICS_H

/* SDR Unit Variables */
#define TOTAL_ACTIVE_POWER "TAP"
#define TOTAL_REACTIVE_POWER "TRP"
#define TOTAL_APPARENT_POWER "TSP"
#define POWER_STATUS "PS"
#define UNIT_ID "UID"
#define UNIT_TAG_LIST "UTL"
#define MODULE_COUNT "MC"
#define CURRENT_TIME "TS"

/* Module Variables */
#define ACTIVE_POWER "AP"
#define REACTIVE_POWER "RP"
#define APPARENT_POWER "SP"
#define VOLTAGE "V"
#define FREQUENCY "FR"
#define POWER_FACTOR "PF"
#define SWITCH_TIME "SWT"
#define CIRCUIT_PRIORITY "CP"
#define MODULE_ID "MID"
#define MODULE_TAG_LIST "MTL"
#define SWITCH_STATUS "MS"

/* Commands */
#define MOD_ON "ON" // ()
#define MOD_OFF "OFF" // ()

#define RE_CLR_QUEUE "CLRQUE" // ()
#define RE_DELAY "DELAY" // ()

#define REQUEST_UPDATE "REQUPD" // ()
#define PUBLISH_READINGS "PUBREAD" // ()
#define NOTIFY "NOTIFY" // ()

#define RESTART_UNIT "RESTART" // ()

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
#define ARRAY_TAG_SUBSET_COMPARISON BOOLEAN_OR

#define ARRAY_SEPARATOR ","
#define COMMAND_SEPARATOR ";"
#define STRING_LITERAL_QUOTATION "\""

#endif