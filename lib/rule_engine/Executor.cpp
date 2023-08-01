#include "Executor.h"
#include "../RTOS_tasks/config.h"

#define EXECUTOR_TAG "EXECUTOR"

void Executor::loopExecutor() {

}

void Executor::load_next_command() {
    if (current_command.empty()){
        current_command = command_list.top().command;
        command_list.pop();
    }

    if(current_command.top().type == SEPARATOR) {
        current_command.pop();
    }

    /* Load command name */
    command_name = current_command.top().lexeme;
    current_command.pop();

    /* Load command parameters */
    parameters.clear();
    while(!current_command.empty()) {
        if(current_command.top().type == RIGHT_PARENTHESES) {
            current_command.pop();
            break;
        }

        if (current_command.top().type == SEPARATOR) current_command.pop();
        parameters.push_back(current_command.top());
        current_command.pop();
    }
}

void Executor::ON() {
    ControlQueueMessage ctrl_message;
    ctrl_message.type = CTRL_OFF;
    ctrl_message.data = nullptr;
    auto ctrl_result = xQueueSendToBack(control_queue, (void *) &ctrl_message, 100 / portTICK_PERIOD_MS);
}

void Executor::OFF() {
    ControlQueueMessage ctrl_message;
    ctrl_message.type = CTRL_OFF;
    ctrl_message.data = nullptr;
    auto ctrl_result = xQueueSendToBack(control_queue, (void *) &ctrl_message, 100 / portTICK_PERIOD_MS);
}

void Executor::RESTART() {
    CommsQueueMessage comms_message;
    comms_message.type = COMMS_PREPARE_RESTART;
    comms_message.data = nullptr;

    ControlQueueMessage ctrl_message;
    ctrl_message.type = CTRL_PREPARE_RESTART;
    ctrl_message.data = nullptr;

    RuleEngineQueueMessage re_message;
    re_message.type = RE_PREPARE_RESTART;
    re_message.data = nullptr;

    auto comms_result = xQueueSendToBack(comms_queue, (void *) &comms_message, 100 / portTICK_PERIOD_MS);
    auto ctrl_result = xQueueSendToBack(control_queue, (void *) &ctrl_message, 100 / portTICK_PERIOD_MS);
    auto re_result = xQueueSendToBack(rule_engine_queue, (void *) &re_message, 100 / portTICK_PERIOD_MS);

    if (comms_result != pdTRUE) {
        ESP_LOGE(EXECUTOR_TAG, "Failed to add message to comms queue.");
    }
    if (ctrl_result != pdTRUE) {
        ESP_LOGE(EXECUTOR_TAG, "Failed to add message to control queue.");
    }
    if (re_result != pdTRUE) {
        ESP_LOGE(EXECUTOR_TAG, "Failed to add message to rule engine queue.");
    }

    return;
}

void Executor::DELAY() {
    if(!wait_for_delay) {
        wait_for_delay = true;
        delay_start_time = esp_timer_get_time();

        return;
    }
    
    uint64_t elapsed_time = esp_timer_get_time() - delay_start_time;
    
    if(delay_time <= (elapsed_time / 1000000 )) {
        next_command = true;
        wait_for_delay = false;
    }
}

void Executor::PUBSTAT() {
    CommsQueueMessage msg;
    msg.type = PUBLISH_STATUS;
    msg.data = nullptr;

    auto status = xQueueSendToBack(comms_queue, (void*) &msg, 100 / portTICK_PERIOD_MS);
    
    if (status != pdTRUE) {
        ESP_LOGE(EXECUTOR_TAG, "Failed to add message to comms queue.");
    }
}

void Executor::REQUPD() {
    CommsQueueMessage msg;
    msg.type = REQUEST_UPDATE;
    msg.data = nullptr;

    auto status = xQueueSendToBack(comms_queue, (void*) &msg, 100 / portTICK_PERIOD_MS);
    
    if (status != pdTRUE) {
        ESP_LOGE(EXECUTOR_TAG, "Failed to add message to comms queue.");
    }
}

void Executor::PUBREAD() {
    ControlQueueMessage msg;
    msg.type = CTRL_READ_MODULES;
    msg.data = nullptr;

    auto status = xQueueSendToBack(control_queue, (void*) &msg, 100 / portTICK_PERIOD_MS);

    if (status != pdTRUE) {
        ESP_LOGE(EXECUTOR_TAG, "Failed to add message to control queue.");
    }
}

void Executor::NOTIFY(ps_string& message) {
    char* temp_msg = (char*) heap_caps_calloc(1, strlen(message.c_str()) + 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    strcpy(temp_msg, message.c_str());

    CommsQueueMessage msg;
    msg.type = NOTIFY_MSG;
    msg.data = (void *) temp_msg;

    auto status = xQueueSendToBack(comms_queue, (void*) &msg, 100 / portTICK_PERIOD_MS);

    if (status != pdTRUE) {
        ESP_LOGE(EXECUTOR_TAG, "Failed to add message to comms queue.");
    }
}
