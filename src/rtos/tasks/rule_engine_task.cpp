#include "tasks.h"

#include "App.h"
#include "config.h"
#include <ps_stl.h>

#include "../sdr/Unit.h"
#include "../sdr/Module.h"

void ruleEngineTaskFunction(void* global_class) {
    std::shared_ptr<sdr::App> app(nullptr);
    { /* Convert nullptr into App pointer, then get a shared pointer of the class, releasing the appClass pointer once it is no longer required. */
        auto appClass = static_cast<sdr::App*>(global_class);
        app = appClass -> get_shared_ptr();
    }

    /* Wait till the task is ready to run. */
    xSemaphoreTake(app -> control_task_semaphore, portMAX_DELAY);
    xSemaphoreGive(app -> control_task_semaphore);

    while(1) {
        /* Check that task has not been paused. */
        xSemaphoreTake(app -> control_task_semaphore, portMAX_DELAY);
        xSemaphoreGive(app -> control_task_semaphore);

        try {
            auto modules = app -> get_modules();
            auto unit = app -> get_unit();

            unit.data().reason();

            for (auto& module : modules.data()) {
                module -> reason();
            }
        } catch (...) {
            ESP_LOGE("Except", "Something went wrong during reasoning.");
        }

        vTaskDelay(10000 / portTICK_PERIOD_MS);
    }
}