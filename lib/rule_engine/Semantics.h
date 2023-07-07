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
#define CIRCUIT_PRIORITY "CP"
#define SWITCH_STATUS "MS"

/* Commands */
#define DIRECT_ON "DON" // (int min_on_s)
#define DIRECT_OFF "DOFF" // (int min_off_s)
#define SCHEDULE_ON "ON" // (int offset_s, int window_s, int min_on_s)
#define SCHEDULE_OFF "OFF" // (int offset_s, int window_s, int min_off_s)
#define ALL_ON "ALL_ON" // (int offset_s, int window_s, int min_on_s)
#define ALL_OFF "ALL_OFF" // (int offset_s, int window_s, int min_off_s)

#define UPDATE_UNIT "UPDUNIT" // (int offset_s, int window_s)

#define PUBLISH_STATUS "PUBSTAT" // (int offset_s, int window_s)
#define PUBLISH_MODULE_READINGS "PUBREAD" // (int offset_s, int window_s)
#define PUBLISH_ALL_READINGS "PUBALL" // (int offset_s, int window_s)

#define DISABLE_RULE_ENGINE "DS_ENGINE" // (int time_s)

#define RESET_UNIT "RESET" // ()
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

#endif