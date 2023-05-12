#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

#define USE_GSM

constexpr size_t BROTLI_DECODER_BUFFER_SIZE = 4096;
constexpr const char * MQTT_STORAGE_NVS_PATH = "/Connectivity/mqtt_nvs";


class SDRException : public std::exception
{
public:
    SDRException(const std::string& message) : message_(message) {}
    const char* what() const noexcept override { return message_.c_str(); }

private:
    std::string message_;
};

#endif