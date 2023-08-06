#pragma once

#ifndef SDR_APP_H
#define SDR_APP_H
#include <Arduino.h>
#include <FastLED.h>
#include <LittleFS.h>
#include <memory>

#include "../Communication/MessageSerializer.h"
#include "../Communication/MessageDeserializer.h"
#include "../Communication/MQTTClient.h"

#include "../rule_engine/Evaluator.h"
#include "../rule_engine/Executor.h"

#include "../sdr_containers/SDRModule.h"
#include "../sdr_containers/SDRUnit.h"

#include "pin_map.h"
#include "../hardware_interface/StatusLED.h"
#include "../hardware_interface/InterfaceMaster.h"

#include "Persistence.h"

#include "../data_containers/ps_string.h"
#include "../data_containers/ps_vector.h"
#include "../data_containers/ps_queue.h"

#include "config.h"

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
 * @brief A simple template class which guards access to a global variable using a semaphore. On construction, the provided mutex is taken. 
 * The data type can then by get/set by reference using the data() method. The semaphore is automatically given back once the class runs out of scope.
 * Cannot be copied.
 * 
 * @tparam DataType The type of data the class should hold.
 */
template <typename DataType>
class VarGuard {
    private:
        SemaphoreHandle_t& _mutex;
        std::shared_ptr<DataType> _data;
    public:
        VarGuard(SemaphoreHandle_t& mutex) :_mutex(mutex), _data(nullptr) {
            xSemaphoreTake(_mutex, portMAX_DELAY);
        }

        /**
         * @brief Returns a read/write reference to the data in the class.
         * 
         * @return DataType& 
         */
        DataType& data() {
            if (!_data) {
                throw std::runtime_error("Data not initialized");
            }
            return *_data;
        }

        std::shared_ptr<DataType> ptr() {
            return _data;
        }

        /**
         * @brief Operator to set the internal shared pointer of the guard class.
         * 
         * @param data - std::shared_ptr of DataType in guard class.
         * @return VarGuard& 
         */
        VarGuard& operator=(const std::shared_ptr<DataType>& data) {
            _data = data;
            return *this;
        }
        
        // Destructor
        ~VarGuard() {
            xSemaphoreGive(_mutex);
        }
};

class AppClass : public StatusLED, public std::enable_shared_from_this<AppClass> {
    private: 
        void initRTOS();
        void deinitRTOS();

        std::shared_ptr<SDRUnit> unit;
        std::shared_ptr<ps_vector<std::shared_ptr<Module>>> modules;
        std::shared_ptr<fs::LittleFSFS> file_system;

    public:
        SemaphoreHandle_t sentry_task_semaphore;
        SemaphoreHandle_t control_task_semaphore;
        SemaphoreHandle_t comms_task_semaphore;
        SemaphoreHandle_t rule_engine_task_semaphore;

        SemaphoreHandle_t unit_mutex;
        SemaphoreHandle_t modules_mutex;
        SemaphoreHandle_t fs_mutex;

        TaskHandle_t sentry_task_handle;
        TaskHandle_t control_task_handle;
        TaskHandle_t comms_task_handle;
        TaskHandle_t rule_engine_task_handle;

        QueueHandle_t sentry_task_queue;
        QueueHandle_t control_task_queue;
        QueueHandle_t comms_task_queue;
        QueueHandle_t rule_engine_task_queue;

        AppClass() : StatusLED(), unit(nullptr), modules(nullptr) {}
        ~AppClass() {
        }

        bool begin();

        VarGuard<SDRUnit> get_unit();
        VarGuard<ps_vector<std::shared_ptr<Module>>> get_modules();
        VarGuard<fs::LittleFSFS> get_fs();

        std::shared_ptr<AppClass> get_shared_ptr() {
            return shared_from_this();
        }

        void set_unit(std::shared_ptr<SDRUnit> _unit) {
            xSemaphoreTake(unit_mutex, portMAX_DELAY);
            unit = _unit;
            xSemaphoreGive(unit_mutex);
        }

};
}
#endif