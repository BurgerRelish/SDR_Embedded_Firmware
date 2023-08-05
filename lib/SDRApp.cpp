#include "SDRApp.h"
#include "HardwareSerial.h"
#include <LITTLEFS.h>

void SDR::AppClass::initRTOS() {
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
    evaluators_mutex = xSemaphoreCreateMutex();

    xSemaphoreGive(unit_mutex);
    xSemaphoreGive(modules_mutex);
    xSemaphoreGive(evaluators_mutex);

    /* Create Global Variable Binary Semaphores */
    executor_binary_semaphore = xSemaphoreCreateBinary();
    interface_binary_semaphore = xSemaphoreCreateBinary();
    mqtt_client_binary_semaphore = xSemaphoreCreateBinary();

    xSemaphoreGive(executor_binary_semaphore);
    xSemaphoreGive(interface_binary_semaphore);
    xSemaphoreGive(mqtt_client_binary_semaphore);

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

bool SDR::AppClass::begin() {
    initRTOS();
    return true;
}

SDR::VarGuard<SDRUnit*> SDR::AppClass::get_unit(){
    auto ret = VarGuard<SDRUnit*>(unit_mutex);
    ret.data() = unit;
    return ret;
}

SDR::VarGuard<ps_vector<Module*>> SDR::AppClass::get_modules(){
    auto ret = VarGuard<ps_vector<Module*>>(modules_mutex);
    ret.data() = modules;
    return ret;
}

SDR::VarGuard<ps_vector<Evaluator*>> SDR::AppClass::get_evaluators(){
    auto ret = VarGuard<ps_vector<Evaluator*>>(evaluators_mutex);
    ret.data() = evaluators;
    return ret;
}

SDR::VarGuard<Executor*> SDR::AppClass::get_executor(){
    auto ret = VarGuard<Executor*>(executor_binary_semaphore);
    ret.data() = executor;
    return ret;
}

SDR::VarGuard<MQTTClient*> SDR::AppClass::get_mqtt_client(){
    auto ret = VarGuard<MQTTClient*>(mqtt_client_binary_semaphore);
    ret.data() = mqtt_client;
    return ret;
}

SDR::VarGuard<InterfaceMaster*> SDR::AppClass::get_interface(){
    auto ret = VarGuard<InterfaceMaster*>(interface_binary_semaphore);
    ret.data() = interface;
    return ret;
}

SDR::VarGuard<fs::LittleFSFS> SDR::AppClass::get_fs() {
    auto ret = VarGuard<fs::LittleFSFS>(fs_binary_semaphore);
    ret.data() = LittleFS;
    return ret;
}