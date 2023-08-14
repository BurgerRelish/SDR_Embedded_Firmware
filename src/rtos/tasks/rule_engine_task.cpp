#include "rtos/tasks.h"

#include <memory>

#include "App.h"
#include "config.h"
#include "VariableDelay.h"

#include "Persistence.h"

#include "App.h"
#include <ps_stl.h>

#include "../sdr/Unit.h"
#include "../sdr/Module.h"

#define TARGET_LOOP_FREQUENCY 10

void create_global_functions(std::shared_ptr<sdr::App>&);

void ruleEngineTaskFunction(void* global_class) {
    std::shared_ptr<sdr::App> app(nullptr);
    { /* Convert nullptr into App pointer, then get a shared pointer of the class, releasing the appClass pointer once it is no longer required. */
        auto appClass = static_cast<sdr::App*>(global_class);
        app = appClass -> get_shared_ptr();
    }

    /* Wait till the task is ready to run. */
    xSemaphoreTake(app -> control_task_semaphore, portMAX_DELAY);
    xSemaphoreGive(app -> control_task_semaphore);



    /* Create a new variable delay class to set target loop frequency. */
    VariableDelay vd("RULE_ENGINE", TARGET_LOOP_FREQUENCY);
    while(1) {
        /* Check that class has not been paused. */
        xSemaphoreTake(app -> control_task_semaphore, portMAX_DELAY);
        xSemaphoreGive(app -> control_task_semaphore);



        /* Wait till target loop time has been reached. */
        vd.delay();
    }
}

void create_global_functions(std::shared_ptr<sdr::App>& app) {
    auto functions = ps::make_shared<re::FunctionStorage>();
}