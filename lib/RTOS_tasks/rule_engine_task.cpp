#include "rule_engine_task.h"
#include "config.h"
#include "VariableDelay.h"
#include "../rule_engine/Evaluator.h"
#include "../rule_engine/Executor.h"

#include "../Persistence.h"

#define TARGET_LOOP_FREQUENCY 10

SDRUnit* unit = nullptr;
ps_vector<Module*> modules;
Evaluator* unit_evaluator = nullptr;
ps_vector<Evaluator*> module_evaluators;
Executor* executor;

void handleREMessage();
void evaluateRE();

void ruleEngineTaskFunction(void* pvParameters);

bool startRuleEngineTask() {
    if (xTaskCreate(
        ruleEngineTaskFunction,
        RULE_ENGINE_TASK_NAME,
        RULE_ENGINE_TASK_STACK,
        NULL,
        RULE_ENGINE_PRIORITY,
        &RuleEngine_Task
        ) == pdTRUE) return true;

    ESP_LOGE("RE", "Failed to start RE task.");
    return false;
}

void ruleEngineTaskFunction(void* pvParameters) {
    VariableDelay vd(RULE_ENGINE_TASK_NAME, TARGET_LOOP_FREQUENCY); // Create a new variable delay class to set target frequency.
    
    vd.addCallback(handleREMessage, 50);

    while (unit == nullptr) { // Wait for at least the unit evaluator to be created.
        vd.loop();
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }

    executor = new Executor(CommsQueue, ControlQueue, RuleEngine_Queue);
    vd.addCallback(evaluateRE, 1000);
    vd.addCallback([]() {return executor -> loopExecutor();}, 100);
    
    while(1) {
        vd.loop();
        vTaskDelay(5 / portTICK_PERIOD_MS);
    }
}

void evaluateRE() {
    Command cmds;
    cmds = unit_evaluator -> evaluate();
    if (!cmds.command.empty()) executor -> addCommand(cmds);

    for (size_t i = 0; i < module_evaluators.size(); i++) {
        cmds = module_evaluators.at(i) -> evaluate();
        if (!cmds.command.empty()) executor -> addCommand(cmds);
    }
}

void handleREMessage() {
    RuleEngineQueueMessage buffer;
    if (xQueueReceive(RuleEngine_Queue, &buffer, 0) != pdTRUE) return;

    switch (buffer.type) {
        case MODULE_CLASS_PTR: {
            modules.push_back((Module*) buffer.data);
            if (modules.size() > 0) {
                auto mod_eval = new Evaluator(unit, modules.back(), ORIG_MOD);
                module_evaluators.push_back(mod_eval);
            }

            break;
        }
        case GLOBAL_CLASS_PTR: {
            unit = (SDRUnit*) buffer.data;

            if (unit_evaluator != nullptr) delete unit_evaluator;
            unit_evaluator = new Evaluator(unit, nullptr, ORIG_UNIT);

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