#ifndef RTOS_STRUCTURE_H
#define RTOS_STRUCTURE_H

#include "stdint.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp32-hal.h"
#include <ps_stl.h>

#define UNIT_UUID "h1u212"

#define RS485_BAUD_RATE 115200

#define deltaHue 1
#define BRIGHTNESS 25

#define WIFI_HOSTNAME "SmartDemand Unit"
#define WIFI_SETUP_AP_PASSWORD "Password123"
#define WIFI_TIMEOUT_MS 120000

#define READING_INTERVAL_MIN 5
#define UPDATE_REQUEST_INTERVAL_MIN 10

#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET 7200

/* Debugging */
#define DEBUG_SENTRY

/* Sentry Task */
#define SENTRY_TASK_NAME "SENTRY"

constexpr uint32_t SENTRY_TASK_STACK = 8096;
constexpr uint8_t SENTRY_QUEUE_SIZE = 10;
constexpr uint8_t SENTRY_PRIORITY = 1;

/* Rule Engine Task */
#define RULE_ENGINE_TASK_NAME "RULE_ENGINE"

constexpr uint32_t RULE_ENGINE_TASK_STACK = 98304;
constexpr uint8_t RULE_ENGINE_QUEUE_SIZE = 10;
constexpr uint8_t RULE_ENGINE_PRIORITY = 3;

/* Communications Task */
#define COMMS_TASK_NAME "COMMS"

constexpr uint32_t COMMS_TASK_STACK = 32768;
constexpr uint8_t COMMS_QUEUE_SIZE = 10;
constexpr uint8_t COMMS_PRIORITY = 4;

/* Module Control Task */
#define CONTROL_TASK_NAME "CONTROL"

constexpr uint32_t CONTROL_TASK_STACK = 16384;
constexpr uint8_t CONTROL_QUEUE_SIZE = 10;
constexpr uint8_t CONTROL_PRIORITY = 2;

#endif