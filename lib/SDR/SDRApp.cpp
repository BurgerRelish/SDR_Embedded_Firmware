#include "SDRApp.h"
#include "HardwareSerial.h"
#include <LITTLEFS.h>
#include "../data_containers/ps_smart_ptr.h"

namespace SDR {

void AppClass::initRTOS() {
    /* Create Task Semaphores */
    sentry_task_semaphore = xSemaphoreCreateBinary();
    control_task_semaphore = xSemaphoreCreateBinary();
    comms_task_semaphore = xSemaphoreCreateBinary();
    rule_engine_task_semaphore = xSemaphoreCreateBinary();

    xSemaphoreTake(sentry_task_semaphore, portMAX_DELAY);
    xSemaphoreTake(control_task_semaphore, portMAX_DELAY);
    xSemaphoreTake(comms_task_semaphore, portMAX_DELAY);
    xSemaphoreTake(rule_engine_task_semaphore, portMAX_DELAY);

    /* Create Global Variable Mutexes */
    unit_mutex = xSemaphoreCreateMutex();
    modules_mutex = xSemaphoreCreateMutex();
    fs_mutex = xSemaphoreCreateMutex();

    xSemaphoreGive(unit_mutex);
    xSemaphoreGive(modules_mutex);
    xSemaphoreGive(fs_mutex);

    /* Create Task Queues */
    xQueueCreate(SENTRY_QUEUE_SIZE, sizeof(SentryQueueMessage));
    xQueueCreate(CONTROL_QUEUE_SIZE, sizeof(ControlQueueMessage));
    xQueueCreate(COMMS_QUEUE_SIZE, sizeof(CommsQueueMessage));
    xQueueCreate(RULE_ENGINE_QUEUE_SIZE, sizeof(RuleEngineQueueMessage));

    /* Create Tasks */

    if (xTaskCreate(
        sentryTaskFunction,
        SENTRY_TASK_NAME,
        SENTRY_TASK_STACK,
        this,
        SENTRY_PRIORITY,
        &sentry_task_handle
    ) != pdTRUE) ESP_LOGE("SETUP", "Failed to start Sentry Task.");

    if (xTaskCreate(
        sentryTaskFunction,
        COMMS_TASK_NAME,
        COMMS_TASK_STACK,
        this,
        COMMS_PRIORITY,
        &comms_task_handle
    ) != pdTRUE) ESP_LOGE("SETUP", "Failed to start Comms Task.");

    if (xTaskCreate(
        sentryTaskFunction,
        CONTROL_TASK_NAME,
        CONTROL_TASK_STACK,
        this,
        CONTROL_PRIORITY,
        &control_task_handle
    ) != pdTRUE) ESP_LOGE("SETUP", "Failed to start Control Task.");

    if (xTaskCreate(
        sentryTaskFunction,
        RULE_ENGINE_TASK_NAME,
        RULE_ENGINE_TASK_STACK,
        this,
        RULE_ENGINE_PRIORITY,
        &rule_engine_task_handle
    ) != pdTRUE) ESP_LOGE("SETUP", "Failed to start Rule Engine Task.");

} 

void AppClass::deinitRTOS() {
    return;
}

bool AppClass::begin() {
    file_system = ps::make_shared<fs::LittleFSFS>();
    file_system -> begin();

    modules = ps::make_shared<ps_vector<std::shared_ptr<Module>>>();
    
    initRTOS();
    return true;
}

VarGuard<SDRUnit> AppClass::get_unit(){
    auto ret = VarGuard<SDRUnit>(unit_mutex);
    ret = unit;
    return ret;
}

VarGuard<ps_vector<std::shared_ptr<Module>>> AppClass::get_modules(){
    auto ret = VarGuard<ps_vector<std::shared_ptr<Module>>>(modules_mutex);
    ret = modules;
    return ret;
}

VarGuard<fs::LittleFSFS> AppClass::get_fs() {
    auto ret = VarGuard<fs::LittleFSFS>(fs_mutex);
    ret = file_system;
    return ret;
}
}
