#ifndef APP_H
#define APP_H

#include <Arduino.h>
#include <../config.h>

#include <../lib/Communication/MQTTClient.h>

class App
{
    public:
        void begin();

    private:
        MQTTClient * mqtt_client = nullptr;
};

#endif