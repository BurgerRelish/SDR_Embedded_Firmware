#include "rule_engine_task.h"
#include "config.h"
#include "VariableDelay.h"
#include "../device_identifiers.h"
#include "../rule_engine/Evaluator.h"
#include "../rule_engine/Executor.h"

#define TARGET_LOOP_FREQUENCY 1

SDRUnit* unit = nullptr;
ps_vector<Module*> modules;
ps_vector<Evaluator*> evaluators;

void handleREMessage(RuleEngineQueueMessage& buffer);

void ruleEngineTaskFunction(void* pvParameters);

void startRuleEngineTask() {
    xTaskCreate(
        ruleEngineTaskFunction,
        RULE_ENGINE_TASK_NAME,
        RULE_ENGINE_TASK_STACK,
        NULL,
        RULE_ENGINE_PRIORITY,
        &RuleEngine_Task
        );
}

void ruleEngineTaskFunction(void* pvParameters) {
    auto unit = SDRUnit(UNIT_UID, "/unit");

    RuleEngineQueueMessage buffer;
    VariableDelay vd(RULE_ENGINE_TASK_NAME, TARGET_LOOP_FREQUENCY); // Create a new variable delay class to set target frequency.
    while(1) {
        if (xQueueReceive(RuleEngine_Queue, &buffer, 0) == pdTRUE) {
            handleREMessage(buffer);
        }



        vd.delay(); // Wait for remainder of frame to reach target frequency.
    }
}


void handleREMessage(RuleEngineQueueMessage& buffer) {
    switch (buffer.type) {
        case MODULE_CLASS_PTR: {
            modules.push_back((Module*) buffer.data);

            break;
        }
        case GLOBAL_CLASS_PTR: {
            unit = (SDRUnit*) buffer.data;
            break;
        }
        case RE_PREPARE_RESTART: {

            break;
        }

        default: {
            break;
        }
    }
}