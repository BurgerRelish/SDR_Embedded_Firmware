#pragma once

#ifndef SDR_APP_H
#define SDR_APP_H
#include <Arduino.h>
#include <FastLED.h>
#include <LittleFS.h>

#include "../Communication/MessageSerializer.h"
#include "../Communication/MessageDeserializer.h"
#include "../Communication/MQTTClient.h"

#include "../rule_engine/Evaluator.h"
#include "../rule_engine/Executor.h"

#include "../sdr_containers/SDRModule.h"
#include "../sdr_containers/SDRUnit.h"

#include "../pin_map.h"
#include "../hardware_interface/StatusLED.h"
#include "../hardware_interface/InterfaceMaster.h"

#include "../Persistence.h"

#include "../data_containers/ps_string.h"
#include "../data_containers/ps_vector.h"
#include "../data_containers/ps_queue.h"

#include "../config.h"

#include "../RTOS_tasks/sentry_task.h"
#include "../RTOS_tasks/rule_engine_task.h"
#include "../RTOS_tasks/communication_task.h"


namespace SDR {

class Exception : public std::exception {
    private:
        ps_string what_str;
    public:
        Exception(const char* message) {
            what_str = message;
        }

        const char* what() {
            return what_str.c_str();
        }
};

/**
 * @brief A simple template class which ensures global variable concurrency. On construction, the provided mutex is taken. The data type can then by get/set by reference
 * using the data() method. The semaphore is automatically given once the class runs out of scope.
 * 
 * @tparam DataType The type of data the class should hold.
 */
template <typename DataType>
class VarGuard {
    private:
        SemaphoreHandle_t _mutex;
        DataType& _data;
    public:
        VarGuard(SemaphoreHandle_t& mutex) :_mutex(mutex) {
            xSemaphoreTake(_mutex, portMAX_DELAY);
        }

        /**
         * @brief Returns a read/write reference to the data in the class.
         * 
         * @return DataType& 
         */
        DataType& data() {
            return _data;
        }

        ~VarGuard() {
            xSemaphoreGive(_mutex);
        }
};

class AppClass : public StatusLED {
    private: 
        void initRTOS();
        void deinitRTOS();

        friend InterfaceMaster;
        void interface_rx_callback(uint8_t interface, std::string message);

        friend MQTTClient;
        void mqtt_client_rx_callback(MessageDeserializer* message);

        SDRUnit* unit = nullptr;
        ps_vector<Module*> modules;
        ps_vector<Evaluator*> evaluators;
        Executor* executor = nullptr;

        MQTTClient* mqtt_client = nullptr;
        InterfaceMaster* interface = nullptr;

    public:
        SemaphoreHandle_t sentry_task_semaphore;
        SemaphoreHandle_t control_task_semaphore;
        SemaphoreHandle_t comms_task_semaphore;
        SemaphoreHandle_t rule_engine_task_semaphore;

        SemaphoreHandle_t unit_mutex;
        SemaphoreHandle_t modules_mutex;
        SemaphoreHandle_t evaluators_mutex;

        SemaphoreHandle_t executor_binary_semaphore;
        SemaphoreHandle_t mqtt_client_binary_semaphore;
        SemaphoreHandle_t interface_binary_semaphore;
        SemaphoreHandle_t fs_binary_semaphore;

        TaskHandle_t sentry_task_handle;
        TaskHandle_t control_task_handle;
        TaskHandle_t comms_task_handle;
        TaskHandle_t rule_engine_task_handle;

        QueueHandle_t sentry_task_queue;
        QueueHandle_t control_task_queue;
        QueueHandle_t comms_task_queue;
        QueueHandle_t rule_engine_task_queue;

        AppClass() : StatusLED() {}
        ~AppClass() {
            deinitRTOS();
            delete unit, executor, mqtt_client, interface;

            for (auto v : modules) {
                delete v;
            }

            for (auto v : evaluators) {
                delete v;
            }
        }

        bool begin();

        VarGuard<SDRUnit*> get_unit();
        VarGuard<ps_vector<Module*>> get_modules();

        VarGuard<ps_vector<Evaluator*>> get_evaluators();
        VarGuard<Executor*> get_executor();

        VarGuard<MQTTClient*> get_mqtt_client();
        VarGuard<InterfaceMaster*> get_interface();
        VarGuard<fs::LittleFSFS> get_fs();

};

}

extern SDR::AppClass SDRApp;
#endif