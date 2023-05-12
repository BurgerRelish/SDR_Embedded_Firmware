#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <TinyGsmClient.h>
#include <string>
#include <iostream>
#include <sstream> // for ostringstream

#include <../config.h>

#include <MessageSerializer.h>
#include <ReadingStore.h>

void taskMQTT(void * parameters);

class MQTTClient : private MessageSerializer
{
    public:
        MQTTClient();
        ~MQTTClient();

        void begin();

        std::string getDeviceName();
        std::string getUsername();
        std::string getPassword();
        std::string getEgressTopic();
        std::vector<std::string> getIngressTopics();

        void setDeviceName(std::string new_value);
        void setUsername(std::string new_value);
        void setPassword(std::string new_value);
        void setEgressTopic(std::string new_value);
        void setIngressTopics(std::vector<std::string> new_values);
        
        void sendMessage(std::string message);
    
    private:
        std::vector<std::string> mqtt_details;
        TaskHandle_t handleMQTT;
        PubSubClient * mqtt_client = nullptr;

        void writeMQTTDetails();
        void fillMQTTDetails();

        friend void taskMQTT(void * parameters);
};



#endif