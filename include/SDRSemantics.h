#pragma once

#ifndef SDR_SEMANTICS_H
#define SDR_SEMANTICS_H

/* Rule Engine Variables - See rule_engine/src/Semantics.h*/
#define LAST_EXECUTION_TIME "last_time"
#define CURRENT_TIME "time"

/* Unit Variables */
#define UNIT_CLASS "unit"
#define TOTAL_ACTIVE_POWER "tot_act_pwr"
#define TOTAL_REACTIVE_POWER "tot_rea_pwr"
#define TOTAL_APPARENT_POWER "tot_app_pwr"
#define MEAN_VOLTAGE "mean_voltage"
#define MEAN_FREQUENCY "mean_freq"
#define MEAN_POWER_FACTOR "mean_pwr_factor"
#define POWER_STATUS "power_status"
#define UNIT_ID "unit_id"
#define UNIT_TAG_LIST "unit_tags"
#define MODULE_COUNT "module_count"
#define KWH_PRICE "kwh_price"

/* Module Variables */
#define MODULE_CLASS "module"

#define ACTIVE_POWER "active_pwr"
#define REACTIVE_POWER "real_pwr"
#define APPARENT_POWER "apparent_pwr"

#define VOLTAGE "voltage"
#define FREQUENCY "frequency"
#define POWER_FACTOR "power factor"
#define SWITCH_TIME "switch_time"
#define CIRCUIT_PRIORITY "priority"
#define MODULE_ID "module_id"
#define MODULE_TAG_LIST "module_tags"
#define SWITCH_STATUS "switch_status"

#define READING_COUNT "num_readings"
#define NEW_READING_COUNT "num_new_readings"

/* Common Commands */
#define SET_VARIABLE "setVar" // (str type, str name, value) , value can be "dbl", "int" or "str" as specified by type

/* Module Commands */
#define SET_MODULE_STATE "setState" // (state)

/* Unit Commands */
#define READ_MODULES "readModules" // ()
#define PUBLISH_READINGS "pubRead" // ()
#define RESTART_UNIT "restart" // ()
#define SLEEP_UNIT "sleep" // (uint64_t time_ms)
#define DELAY_UNIT "delay" // (uint64_t time_ms)


#endif