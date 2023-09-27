#pragma once

#ifndef JSON_FIELDS_H
#define JSON_FIELDS_H

/**
 * @brief The Unique User ID of the Control Unit.
*/
#define JSON_UNIT_UID "unit_id"

/**
 * @brief The Unique User ID of the Module.
*/
#define JSON_MODULE_UID "module_id"

/**
 * @brief The type of data contained in a message.
 * @typedef int
 * 
 * @note 0 - Reading Message Type
 */
#define JSON_TYPE "type"

/**
 * @brief The data object of a message.
 */
#define JSON_DATA "data"

// Rule Engine & Tags

#define JSON_PRIORITY "priority"
#define JSON_EXPRESSION "expression"
#define JSON_COMMAND "command"

#define JSON_TAGS "tags"
#define JSON_RULES "rules"

#define JSON_ACTION "action"

#define JSON_APPEND "ap"
#define JSON_REPLACE "rp"
#define JSON_EXECUTE 3
#define JSON_EXECUTE_IF 4

// Serialization

#define JSON_VOLTAGE "mean_voltage"
#define JSON_FREQUENCY "mean_frequency"
#define JSON_APPARENT_POWER "apparent_power"
#define JSON_POWER_FACTOR "power_factor"
#define JSON_READING_COUNT "sample_count"
#define JSON_TIMESTAMP "timestamp"
#define JSON_KWH_USAGE "kwh_usage"

#define JSON_STATUS_OBJ "state_changes"
#define JSON_STATUS "state"

/**
 * @brief The epoch timestamp of the start of this reading period.
 * 
 */
#define JSON_PERIOD_START "period_start"

/**
 * @brief The epoch timestamp of the end of this reading period. (Typically now)
 * 
 */
#define JSON_PERIOD_END "period_end"

#define JSON_READING_OBJECT "readings"
#define JSON_NEW_READINGS "nr"

#endif