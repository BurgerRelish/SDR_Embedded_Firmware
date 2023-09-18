#pragma once

#ifndef SDR_SEMANTICS_H
#define SDR_SEMANTICS_H

/* Rule Engine Variables - See rule_engine/src/Semantics.h*/
#define LAST_EXECUTION_TIME "LEX"
#define CURRENT_TIME "TS"

/* Unit Variables */
#define UNIT_CLASS "UNIT"
#define TOTAL_ACTIVE_POWER "TAP"
#define TOTAL_REACTIVE_POWER "TRP"
#define TOTAL_APPARENT_POWER "TSP"
#define MEAN_VOLTAGE "MNV"
#define MEAN_FREQUENCY "MNF"
#define MEAN_POWER_FACTOR "MPF"
#define POWER_STATUS "PS"
#define UNIT_ID "UID"
#define UNIT_TAG_LIST "UTL"
#define MODULE_COUNT "MC"

/* Module Variables */
#define MODULE_CLASS "MODULE"

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

/* Common Commands */
#define SET_VARIABLE "setVar" // (type, value) , type can be "dbl", "int" or "str"

/* Module Commands */
#define SET_MODULE_STATE "setState" // (state)
#define READ_MODULE_DATA "readModule" // () 

/* Unit Commands */
#define PUBLISH_READINGS "pubRead" // ()
#define REQUEST_UPDATE "reqUpdate" // ()
#define RESTART_UNIT "restart" // ()
#define SLEEP_UNIT "sleep" // (uint64_t time_us)

#define NOTIFY "NOTIFY" // ()



#endif