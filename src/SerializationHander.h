#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <ps_stl.h>

#include "./App/Unit.h"
#include "MQTTClient.h"

class SerializationHandler {
    private:
    std::shared_ptr<Unit> unit;
    std::shared_ptr<MQTTClient> mqtt_client;

    public:
    SerializationHandler();
    void begin(std::shared_ptr<Unit> unit, std::shared_ptr<MQTTClient> mqtt_client);

    void serializeReadings();
};