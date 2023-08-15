#pragma once

#ifndef RTOS_TASKS_H
#define RTOS_TASKS_H

enum SentryTaskState {
    COMMS_SETUP_COMPLETE,
    CTRL_SETUP_COMPLETE,
    RE_SETUP_COMPLETE
};

struct SentryQueueMessage {
    SentryTaskState new_state;
    void* data;
};

enum RuleEngineMessageType {
    RE_UNIT_COMMAND,
    RE_MODULE_COMMAND
};

struct RuleEngineQueueMessage {
    RuleEngineMessageType type;
    std::shared_ptr<void> data;
};

enum ControlMessageType {
    CTRL_UNIT_COMMAND,
    CTRL_MODULE_COMMAND
};

struct ControlQueueMessage {
    ControlMessageType type;
    std::shared_ptr<void> data;
};

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


void sentryTaskFunction(void* pvParameters);
void commsTaskFunction(void* global_class);
void ruleEngineTaskFunction(void* global_class);
void controlTaskFunction(void* global_class);

#endif