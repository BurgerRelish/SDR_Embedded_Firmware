#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <string>

#include <MessageSerializer.h>
#include <ReadingStore.h>


class MQTTClient : private MessageSerializer
{
    public:
        MQTTClient();
        ~MQTTClient();

        void begin();
        
        void sendMessage(std::string message);
        
    private:

    protected:

};



#endif