#include "Executor.h"
#include "../RTOS_tasks/config.h"

#define EXECUTOR_TAG "EXECUTOR"

void Executor::loopExecutor() {
    if (next_command) load_next_command();

    if (command_name == RE_DELAY) {
        /* Load delay period */
        if (!wait_for_delay) {
            auto parameter_token = parameters.at(0);
            if (parameter_token.type != NUMERIC_LITERAL) {
                ESP_LOGE(EXECUTOR_TAG, "Invalid parameter format for delay, skipping.");
                next_command = true;
                return;
            }

            delay_time = (int) std::atof(parameter_token.lexeme.c_str());
        }

        DELAY();
    } else if (command_name == RE_CLR_QUEUE) {
        CLRQUE();
    } else if (command_name == MOD_ON) {
        ON();
    } else if (command_name == MOD_OFF) {
        OFF();
    } else if (command_name == REQUEST_UPDATE) {
        auto parameter_token = parameters.at(0);
        if (parameter_token.type != NUMERIC_LITERAL) {
            ESP_LOGE(EXECUTOR_TAG, "Invalid parameter format for update request, skipping.");
            next_command = true;
            return;
        }

        REQUPD((int) std::atof(parameter_token.lexeme.c_str()));
    } else if (command_name == PUBLISH_READINGS) {
        auto parameter_token = parameters.at(0);
        if (parameter_token.type != NUMERIC_LITERAL) {
            ESP_LOGE(EXECUTOR_TAG, "Invalid parameter format for reading publish, skipping.");
            next_command = true;
            return;
        }

        PUBREAD((int) std::atof(parameter_token.lexeme.c_str()));
    } else if (command_name == NOTIFY) {
        auto parameter_token = parameters.at(0);
        if (parameter_token.type != STRING_LITERAL) {
            ESP_LOGE(EXECUTOR_TAG, "Invalid parameter format for notify, skipping.");
            next_command = true;
            return;
        }

        COMMS_NOTIFY(parameter_token.lexeme);
    } else if (command_name == RESTART_UNIT) {
        RESTART();
    }
}

void Executor::load_next_command() {
    if (current_command.empty()){ // Load a new rule command from the priority list.
        current_command = command_list.top().command;
        cmd_origin = command_list.top().origin;
        cmd_origin_type = command_list.top().type;
        command_list.pop();
    }

    if(current_command.front().type == SEPARATOR) {
        current_command.pop();
    }

    /* Load command name */
    command_name = current_command.front().lexeme;
    current_command.pop();

    /* Load command parameters */
    parameters.clear();
    while(!current_command.empty()) {
        if(current_command.front().type == RIGHT_PARENTHESES) {
            current_command.pop();
            break;
        }

        if (current_command.front().type == SEPARATOR) current_command.pop();
        parameters.push_back(current_command.front());
        current_command.pop();
    }

    next_command = false;
}

void Executor::ON() {
    ControlQueueMessage ctrl_message;
    ctrl_message.type = CTRL_ON;
    ctrl_message.data = (void*) cmd_origin.module;
    auto ctrl_result = xQueueSendToBack(control_queue, (void *) &ctrl_message, 100 / portTICK_PERIOD_MS);

    if (ctrl_result != pdTRUE) {
        ESP_LOGE(EXECUTOR_TAG, "Failed to add message to control queue.");
    }
    next_command = true;
}

void Executor::OFF() {
    ControlQueueMessage ctrl_message;
    ctrl_message.type = CTRL_OFF;
    ctrl_message.data = (void*) cmd_origin.module;
    auto ctrl_result = xQueueSendToBack(control_queue, (void *) &ctrl_message, 100 / portTICK_PERIOD_MS);

    if (ctrl_result != pdTRUE) {
        ESP_LOGE(EXECUTOR_TAG, "Failed to add message to control queue.");
    }
    next_command = true;
}

void Executor::RESTART() {
    CommsQueueMessage comms_message;
    comms_message.type = MSG_COMMS_PREPARE_RESTART;
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

    next_command = true;
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

void Executor::PUBSTAT(int window) {
    CommsQueueMessage msg;
    msg.type = MSG_PUBLISH_STATUS;
    auto data_ptr = (int*) heap_caps_malloc(sizeof(int), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    *data_ptr = window;
    msg.data = data_ptr;

    auto status = xQueueSendToBack(comms_queue, (void*) &msg, 100 / portTICK_PERIOD_MS);
    
    if (status != pdTRUE) {
        ESP_LOGE(EXECUTOR_TAG, "Failed to add message to comms queue.");
    }

    next_command = true;
}

void Executor::REQUPD(int window) {
    CommsQueueMessage msg;
    msg.type = MSG_REQUEST_UPDATE;

    auto data_ptr = (int*) heap_caps_malloc(sizeof(int), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    *data_ptr = window;
    msg.data = data_ptr;

    auto status = xQueueSendToBack(comms_queue, (void*) &msg, 100 / portTICK_PERIOD_MS);
    
    if (status != pdTRUE) {
        ESP_LOGE(EXECUTOR_TAG, "Failed to add message to comms queue.");
    }

    next_command = true;
}

void Executor::PUBREAD(int window) {
    ControlQueueMessage msg;
    msg.type = CTRL_READ_MODULES;
    auto data_ptr = (int*) heap_caps_malloc(sizeof(int), MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    *data_ptr = window;
    msg.data = data_ptr;

    auto status = xQueueSendToBack(control_queue, (void*) &msg, 100 / portTICK_PERIOD_MS);

    if (status != pdTRUE) {
        ESP_LOGE(EXECUTOR_TAG, "Failed to add message to control queue.");
    }

    next_command = true;
}

void Executor::COMMS_NOTIFY(ps_string& message) {
    char* temp_msg = (char*) heap_caps_calloc(1, strlen(message.c_str()) + 1, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
    strcpy(temp_msg, message.c_str());

    CommsQueueMessage msg;
    msg.type = NOTIFY_MSG;
    msg.data = (void *) temp_msg;

    auto status = xQueueSendToBack(comms_queue, (void*) &msg, 100 / portTICK_PERIOD_MS);

    if (status != pdTRUE) {
        ESP_LOGE(EXECUTOR_TAG, "Failed to add message to comms queue.");
    }

    next_command = true;
}
