#include <memory>

#include "rule_engine_task.h"
#include "config.h"
#include "VariableDelay.h"
#include "../rule_engine/Evaluator.h"
#include "../rule_engine/Executor.h"

#include "Persistence.h"

#include "SDRApp.h"
#include "../ps_stl/ps_stl.h"


#define TARGET_LOOP_FREQUENCY 10

void handleREMessage(std::shared_ptr<SDR::AppClass>);
void evaluateRE(std::shared_ptr<Evaluator>, std::shared_ptr<ps::vector<std::shared_ptr<Evaluator>>>, std::shared_ptr<Executor>);

void ruleEngineTaskFunction(void* global_class) {
    /* Init Local Shared Pointers. */
    std::shared_ptr<Evaluator> unit_evaluator = nullptr;
    std::shared_ptr<ps::vector<std::shared_ptr<Evaluator>>> module_evaluators;
    std::shared_ptr<Executor> executor = nullptr;

    std::shared_ptr<SDR::AppClass> app(nullptr);
    { /* Convert nullptr into AppClass pointer, then get a shared pointer of the class, releasing the appClass pointer once it is no longer required. */
        auto appClass = static_cast<SDR::AppClass*>(global_class);
        app = appClass -> get_shared_ptr();
    }

    /* Wait till the task is ready to run. */
    xSemaphoreTake(app -> control_task_semaphore, portMAX_DELAY);
    xSemaphoreGive(app -> control_task_semaphore);

    { /* Create an evaluator for the unit as well as each module on the device. */
        auto unit = app -> get_unit();
        unit_evaluator = ps::make_shared<Evaluator>(unit.ptr(), std::shared_ptr<Module>(nullptr), ORIG_UNIT);

        auto modules = app -> get_modules();
        for (auto module : modules.data()) {
            module_evaluators -> push_back(ps::make_shared<Evaluator>(unit.ptr(), module, ORIG_MOD));
        }
    }

    /* Create the command executor */
    executor = ps::make_shared<Executor>(app -> comms_task_queue, app -> control_task_queue, app -> rule_engine_task_queue);

    /* Create a new variable delay class to set target loop frequency. */
    VariableDelay vd("RULE_ENGINE", TARGET_LOOP_FREQUENCY);
    while(1) {
        /* Check that class has not been paused. */
        xSemaphoreTake(app -> control_task_semaphore, portMAX_DELAY);
        xSemaphoreGive(app -> control_task_semaphore);

        /* Receive and handle messages */
        handleREMessage(app);
        
        /* Wait for control over global variables. */
        app -> get_unit();
        app -> get_modules();

        /* Evaluate rules. */
        evaluateRE(unit_evaluator, module_evaluators, executor);

        /* Wait till target loop time has been reached. */
        vd.delay();
    }
}

void evaluateRE(std::shared_ptr<Evaluator> unit_evaluator, std::shared_ptr<ps::vector<std::shared_ptr<Evaluator>>> module_evaluators, std::shared_ptr<Executor> executor) {
    Command cmds;
    cmds = unit_evaluator -> evaluate();
    if (!cmds.command.empty()) executor -> addCommand(cmds);
    
    for (auto module_evaluator : *module_evaluators) {
        cmds = module_evaluator -> evaluate();
        if (!cmds.command.empty()) executor -> addCommand(cmds);
    }
}

void handleREMessage(std::shared_ptr<SDR::AppClass> app) {
    RuleEngineQueueMessage buffer;
    if (xQueueReceive(app -> rule_engine_task_queue, &buffer, 0) != pdTRUE) return;

    switch (buffer.type) {
        case MODULE_CLASS_PTR: {
        }
        case GLOBAL_CLASS_PTR: {
        }
        case RE_PREPARE_RESTART: {
            
            break;
        }
        default: {
            break;
        }
    }

}