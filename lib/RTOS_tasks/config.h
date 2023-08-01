#ifndef RTOS_STRUCTURE_H
#define RTOS_STRUCTURE_H

#include "stdint.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "esp32-hal.h"

/* Connectivity */
#define DEFAULT_WIFI_SSID "Spencer WiFi"
/* Debugging */
#define DEBUG_SENTRY

#define CONNECTIVITY_STORAGE_NVS_PATH "/comms"
/* NVS */
#define MQTT_STORAGE_NVS_PATH "/mqtt"
#define MODULE_STORAGE_NVS_PATH "/modules"
#define UNIT_STORAGE_NVS_PATH "/unit"



/* Sentry Task */
#define SENTRY_TASK_NAME "SENTRY"

xTaskHandle SentryTask = NULL;

constexpr uint32_t SENTRY_TASK_STACK = 32768;
constexpr uint8_t SENTRY_QUEUE_SIZE = 10;
constexpr uint8_t SENTRY_PRIORITY = 1;

enum SentryTaskState {
    STATUS_BOOTING,
    STATUS_GOOD,
    STATUS_NO_COMMS,
    STATUS_DISCONNECT_SCHEDULED,
    STATUS_RECONNECT_SCHEDULED,
    RE_RESTART_READY,
    COMMS_RESTART_READY,
    CTRL_RESTART_READY

};

struct SentryQueueMessage {
    SentryTaskState new_state;
    void* data;
};

xQueueHandle SentryQueue = NULL;

/* Rule Engine Task */
#define RULE_ENGINE_TASK_NAME "RULE_ENGINE"

xTaskHandle RuleEngine_Task = NULL;

xSemaphoreHandle RuleEngine_Mutex = NULL;
xSemaphoreHandle RuleEngineTaskSemaphore = NULL;

constexpr uint32_t RULE_ENGINE_TASK_STACK = 32768;
constexpr uint8_t RULE_ENGINE_QUEUE_SIZE = 10;
constexpr uint8_t RULE_ENGINE_PRIORITY = 3;

enum RuleEngineMessageType {
    MODULE_CLASS_PTR,
    GLOBAL_CLASS_PTR,
    RE_PREPARE_RESTART
};

struct RuleEngineQueueMessage {
    RuleEngineMessageType type;
    void* data;
};

xQueueHandle RuleEngine_Queue = NULL;

/* Communications Task */
#define COMMS_TASK_NAME "COMMS"

xTaskHandle CommsTask = NULL;

xSemaphoreHandle CommsTaskSemaphore = NULL;

constexpr uint32_t COMMS_TASK_STACK = 32768;
constexpr uint8_t COMMS_QUEUE_SIZE = 10;
constexpr uint8_t COMMS_PRIORITY = 4;

enum CommsMessageType {
    MSG_PUBLISH_READINGS,
    MSG_PUBLISH_STATUS,
    MSG_REQUEST_UPDATE,
    NOTIFY_MSG,
    MSG_COMMS_PREPARE_RESTART
};

struct CommsQueueMessage {
    CommsMessageType type;
    void* data;
};

xQueueHandle CommsQueue = NULL;

/* Module Control Task */
#define CONTROL_TASK_NAME "CONTROL"

xTaskHandle ControlTask = NULL;

xSemaphoreHandle ControlTaskSemaphore = NULL;

constexpr uint32_t CONTROL_TASK_STACK = 32768;
constexpr uint8_t CONTROL_QUEUE_SIZE = 10;
constexpr uint8_t CONTROL_PRIORITY = 2;

enum ControlMessageType {
    CTRL_READ_MODULES,
    CTRL_ON,
    CTRL_OFF,
    CTRL_PREPARE_RESTART
};

struct ControlQueueMessage {
    ControlMessageType type;
    void* data;
};

xQueueHandle ControlQueue = NULL;

#endif