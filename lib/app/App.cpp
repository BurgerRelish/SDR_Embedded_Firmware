#include "App.h"
#include "HardwareSerial.h"
#include "FS.h"
#include <LittleFS.h>
#include "config.h"

namespace sdr {

void App::initRTOS() {
    /* Create Task Semaphores */
    sentry_task_semaphore = xSemaphoreCreateBinary();
    control_task_semaphore = xSemaphoreCreateBinary();
    comms_task_semaphore = xSemaphoreCreateBinary();
    rule_engine_task_semaphore = xSemaphoreCreateBinary();

    xSemaphoreTake(sentry_task_semaphore, 1 / portTICK_PERIOD_MS);
    xSemaphoreTake(control_task_semaphore, 1 / portTICK_PERIOD_MS);
    xSemaphoreTake(comms_task_semaphore, 1 / portTICK_PERIOD_MS);
    xSemaphoreTake(rule_engine_task_semaphore, 1 / portTICK_PERIOD_MS);

    /* Create Global Variable Mutexes */
    unit_mutex = xSemaphoreCreateMutex();
    modules_mutex = xSemaphoreCreateMutex();
    fs_mutex = xSemaphoreCreateMutex();
    time_mutex = xSemaphoreCreateMutex();

    xSemaphoreGive(unit_mutex);
    xSemaphoreGive(modules_mutex);
    xSemaphoreGive(fs_mutex);
    xSemaphoreGive(time_mutex);

    /* Create Task Queues */
    sentry_task_queue = xQueueCreate(SENTRY_QUEUE_SIZE, sizeof(SentryQueueMessage));
    control_task_queue = xQueueCreate(CONTROL_QUEUE_SIZE, sizeof(ControlQueueMessage));
    comms_task_queue = xQueueCreate(COMMS_QUEUE_SIZE, sizeof(CommsQueueMessage));
    rule_engine_task_queue = xQueueCreate(RULE_ENGINE_QUEUE_SIZE, sizeof(RuleEngineQueueMessage));

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
        commsTaskFunction,
        COMMS_TASK_NAME,
        COMMS_TASK_STACK,
        this,
        COMMS_PRIORITY,
        &comms_task_handle
    ) != pdTRUE) ESP_LOGE("SETUP", "Failed to start Comms Task.");

    if (xTaskCreate(
        controlTaskFunction,
        CONTROL_TASK_NAME,
        CONTROL_TASK_STACK,
        this,
        CONTROL_PRIORITY,
        &control_task_handle
    ) != pdTRUE) ESP_LOGE("SETUP", "Failed to start Control Task.");

    if (xTaskCreate(
        ruleEngineTaskFunction,
        RULE_ENGINE_TASK_NAME,
        RULE_ENGINE_TASK_STACK,
        this,
        RULE_ENGINE_PRIORITY,
        &rule_engine_task_handle
    ) != pdTRUE) ESP_LOGE("SETUP", "Failed to start Rule Engine Task.");

    ESP_LOGI("APPCLASS", "RTOS Tasks created successfully, starting Sentry...");
    xSemaphoreGive(sentry_task_semaphore);
} 

void App::deinitRTOS() {
    return;
}

bool App::begin() {
    if(!LittleFS.begin(true)) ESP_LOGE("FS", "Failed to mount LittleFS.");
    file_system = std::shared_ptr<fs::LittleFSFS>(&LittleFS);

    modules = ps::make_shared<ps::vector<std::shared_ptr<Module>>>();
    
    initRTOS();
    return true;
}

VarGuard<Unit> App::get_unit(){
    VarGuard<Unit> guard(unit_mutex);
    guard = unit;
    return guard;
}

VarGuard<ps::vector<std::shared_ptr<Module>>> App::get_modules(){
    VarGuard<ps::vector<std::shared_ptr<Module>>> guard(modules_mutex);
    guard = modules;
    return guard;
}

VarGuard<fs::LittleFSFS> App::get_fs() {
    VarGuard<fs::LittleFSFS> guard(fs_mutex);
    guard = file_system;
    return guard;
}

VarGuard<ModuleMap> App::get_module_map() {
    VarGuard<ModuleMap> guard(modules_mutex);
    guard = module_map;
    return guard;
}

std::shared_ptr<App> App::get_shared_ptr() {
    return shared_from_this();
}


bool App::generate_module_map() {
    bool ret = false;
    xSemaphoreTake(modules_mutex, portMAX_DELAY);
    module_map = ps::make_shared<ModuleMap>();
    if (modules -> size() != 0) {
        for (size_t i = 0; i < modules -> size(); i++) {
            module_map -> insert(
                std::make_pair(
                    std::string(modules -> at(i) -> id().c_str()),
                    modules -> at(i)
                )
            );
        }
    }
    xSemaphoreGive(modules_mutex);
    return ret;
}

void App::configure_time() {
    xSemaphoreTake(time_mutex, portMAX_DELAY);
    configTime(GMT_OFFSET, 0, NTP_SERVER);
    xSemaphoreGive(time_mutex);
}

uint64_t App::get_epoch_time() {
    xSemaphoreTake(time_mutex, portMAX_DELAY);

    time_t now;
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        ESP_LOGE("Time", "Failed to get time.");
        return 0;
    }

    time(&now);

    xSemaphoreGive(time_mutex);
    return (uint64_t) now;
}

struct tm App::get_local_time() {
    xSemaphoreTake(time_mutex, portMAX_DELAY);
    struct tm timeinfo;
    
    if(!getLocalTime(&timeinfo)){
        ESP_LOGE("Time", "Failed to get time.");
        return tm();
    }

    xSemaphoreGive(time_mutex);
    return timeinfo;
}

void create_module(ps::string& id,) {

}


}
