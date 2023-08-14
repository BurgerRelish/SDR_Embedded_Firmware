#pragma once

#ifndef SDR_APP_H
#define SDR_APP_H
#include <Arduino.h>
#include <FastLED.h>
#include <LittleFS.h>
#include <ps_stl.h>
#include "time.h"

#include "config.h"
#include "pin_map.h"
#include "Persistence.h"
#include "../hardware_interface/StatusLED.h"
#include "../hardware_interface/InterfaceMaster.h"



#include "rtos/tasks.h"

#include "../sdr/Unit.h"
#include "../sdr/Module.h"

namespace sdr {

class Exception : public std::exception {
    private:
        ps::string what_str;
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

using ModuleMap = std::unordered_map<std::string, std::shared_ptr<Module>>;

class App : public StatusLED, public std::enable_shared_from_this<App> {
    private: 
        void initRTOS();
        void deinitRTOS();

        std::shared_ptr<sdr::Unit> unit;
        std::shared_ptr<ps::vector<std::shared_ptr<Module>>> modules;
        std::shared_ptr<ModuleMap> module_map;
        std::shared_ptr<fs::LittleFSFS> file_system;

    public:
        SemaphoreHandle_t sentry_task_semaphore;
        SemaphoreHandle_t control_task_semaphore;
        SemaphoreHandle_t comms_task_semaphore;
        SemaphoreHandle_t rule_engine_task_semaphore;

        SemaphoreHandle_t unit_mutex;
        SemaphoreHandle_t modules_mutex;
        SemaphoreHandle_t fs_mutex;
        SemaphoreHandle_t time_mutex;

        TaskHandle_t sentry_task_handle;
        TaskHandle_t control_task_handle;
        TaskHandle_t comms_task_handle;
        TaskHandle_t rule_engine_task_handle;

        QueueHandle_t sentry_task_queue;
        QueueHandle_t control_task_queue;
        QueueHandle_t comms_task_queue;
        QueueHandle_t rule_engine_task_queue;

        App() : StatusLED(), unit(nullptr) {}
        ~App() {
        }

        bool begin();
        bool generate_module_map();
        void set_unit(std::shared_ptr<sdr::Unit> _unit);

        VarGuard<sdr::Unit> get_unit();
        VarGuard<ps::vector<std::shared_ptr<Module>>> get_modules();
        VarGuard<fs::LittleFSFS> get_fs();
        VarGuard<ModuleMap> get_module_map();

        std::shared_ptr<App> get_shared_ptr();

        void configure_time(uint32_t gmtOffsetSec, int daylightOffsetSec, const char* ntp_server);
        uint64_t get_epoch_time();
        struct tm get_local_time();

};
}
#endif