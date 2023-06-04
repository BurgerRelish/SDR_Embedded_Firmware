#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

#define USE_GSM

constexpr size_t BROTLI_DECODER_BUFFER_SIZE = 4096;
constexpr const char * MQTT_STORAGE_NVS_PATH = "/Connectivity/mqtt_nvs";


/* RTOS Config */
constexpr uint32_t APP_TASK_STACK = 32768;
constexpr uint32_t CONTROL_TASK_STACK = 32768;
constexpr uint32_t AST_TASK_STACK = 32768;
constexpr uint32_t COMMS_TASK_STACK = 32768;

constexpr uint8_t APP_QUEUE_SIZE = 10;
constexpr uint8_t CONTROL_QUEUE_SIZE = 10;
constexpr uint8_t AST_QUEUE_SIZE = 10;
constexpr uint8_t COMMS_QUEUE_SIZE = 10;


class SDRException : public std::exception
{
public:
    SDRException(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }

private:
    std::string message_;
};

#endif