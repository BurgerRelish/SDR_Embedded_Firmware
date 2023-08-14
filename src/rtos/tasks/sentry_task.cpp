#include "rtos/tasks.h"
#include "config.h"

#include <Arduino.h>
#include <ps_stl.h>

#include "App.h"

#include "VariableDelay.h"

#define TARGET_LOOP_FREQUENCY 10

void checkTaskStatus(TaskHandle_t task);
void bootSequence(std::shared_ptr<sdr::App>&);

void sentryPrintInfo(std::shared_ptr<sdr::App> app) {

    ESP_LOGI("SENTRY", "\n======= Task Information =======");
    log_printf("|    Task     | State |  Stack |");
    log_printf("\n|    Sentry   |   %d   | %5uB |",(int)eTaskGetState(app -> sentry_task_handle), (uint32_t)uxTaskGetStackHighWaterMark(app -> sentry_task_handle));
    log_printf("\n|    Comms    |   %d   | %5uB |",(int)eTaskGetState(app -> comms_task_handle), (uint32_t)uxTaskGetStackHighWaterMark(app -> comms_task_handle));
    log_printf("\n|   Control   |   %d   | %5uB |",(int)eTaskGetState(app -> control_task_handle), (uint32_t)uxTaskGetStackHighWaterMark(app -> control_task_handle));
    log_printf("\n| Rule Engine |   %d   | %5uB |",(int)eTaskGetState(app -> rule_engine_task_handle), (uint32_t)uxTaskGetStackHighWaterMark(app -> rule_engine_task_handle));
    log_printf("\n--------------------------------");

    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t tot_psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    size_t free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);
    size_t tot_sram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);
    log_printf("\n- PSRAM Usage: %f%%", 100 *( 1 - ((float) free_psram) / ((float) tot_psram)));
    log_printf("\n- SRAM Usage: %f%%\n\n", 100 * (1 - ((float) free_sram) / ((float) tot_sram)));
}

void sentryTaskFunction(void* global_class) {
    std::shared_ptr<sdr::App> app;
    {
        auto appClass = static_cast<sdr::App*>(global_class);
        app = appClass -> get_shared_ptr();
    }

    try {
        xSemaphoreTake(app -> sentry_task_semaphore, portMAX_DELAY);
        xSemaphoreGive(app -> sentry_task_semaphore);
        app -> setStatusLEDState(STATUS_LED_SETUP);
        app -> updateStatusLED();

        /* Allow other RTOS tasks to start. */
        bootSequence(app);

        VariableDelay vd(SENTRY_TASK_NAME, TARGET_LOOP_FREQUENCY); // Create a new variable delay class to set target frequency.
        vd.addCallback([app](){sentryPrintInfo(app);}, (uint32_t)2500);
        app -> setStatusLEDState(STATUS_LED_RUNNING);
        while(1) {
            xSemaphoreTake(app -> sentry_task_semaphore, portMAX_DELAY);
            xSemaphoreGive(app -> sentry_task_semaphore);
            
            if(!app -> updateStatusLED()) ESP_LOGE("SENTRY", "Failed to update Status LED.");

            checkTaskStatus(app -> comms_task_handle);
            checkTaskStatus(app -> control_task_handle);
            checkTaskStatus(app -> rule_engine_task_handle);
            
            vd.loop();
        }
    } catch (sdr::Exception &e) {
        ESP_LOGE("SENTRY", "sdr Exception: %s", e.what());
    } catch (std::exception &e) {
        ESP_LOGE("SENTRY", "Exception: %s", e.what());
    }

    vTaskDelete(NULL);
}

void checkTaskStatus(TaskHandle_t task) {
    auto status = eTaskGetState(task);

    switch (status) {
        case eRunning:
            // Task is currently running
            break;
        case eReady:
            // Task is ready to run but not currently executing
            break;
        case eBlocked:
            // Task is blocked, waiting for a resource or event
            break;
        case eSuspended:
            // Task is suspended
            break;
        case eDeleted:
            // Task has been deleted
            break;
        case eInvalid:
            // Task handle is invalid or task does not exist
            break;
    }
}

void bootSequence(std::shared_ptr<sdr::App>& app) {
    /* Start Comms Task and Wait for communications setup to complete. */
    ESP_LOGI("BOOT", "Resuming Comms Task.");
    if(xSemaphoreGive(app -> comms_task_semaphore) != pdTRUE) ESP_LOGE("BOOT", "Failed to release comms semaphore.");
    while (1) {
        SentryQueueMessage msg;
        if (xQueueReceive(app -> sentry_task_queue, &msg, 50 / portTICK_PERIOD_MS) != pdTRUE) { // Update the status LED every 50ms while waiting.
            app -> updateStatusLED();
            continue;
        }

        ESP_LOGE("BOOT", "Got Message: %d", msg.new_state);
        if (msg.new_state == COMMS_SETUP_COMPLETE) break;
    }

    // /* Start Control Task and wait for setup to complete. */
    // xSemaphoreGive(app -> control_task_semaphore);
    // while (1) {
    //     if (xQueueReceive(app -> sentry_task_queue, &msg, 100 / portTICK_PERIOD_MS) != pdTRUE) { // Update the status LED every 100ms while waiting.
    //         app -> updateStatusLED();
    //         continue;
    //     }
    //     if (msg.new_state != CTRL_SETUP_COMPLETE) ESP_LOGE("SENTRY", "Got Unknown Message: %d", msg.new_state);
    //     else break;
    // }
    // vTaskResume(app -> comms_task_handle); // Resume the comms task once the control task has been set up.

    // /* Start the Rule Engine task and wait for setup to complete. */
    // xSemaphoreGive(app -> rule_engine_task_semaphore);
    // while (1) {
    //     if (xQueueReceive(app -> sentry_task_queue, &msg, 100 / portTICK_PERIOD_MS) != pdTRUE) { // Update the status LED every 100ms while waiting.
    //         app -> updateStatusLED();
    //         continue;
    //     }
    //     if (msg.new_state != RE_SETUP_COMPLETE) ESP_LOGE("SENTRY", "Got Unknown Message: %d", msg.new_state);
    //     else break;
    // }
    // vTaskResume(app -> control_task_handle); // Resume the control task once both the rule engine and comms tasks are running.
}


