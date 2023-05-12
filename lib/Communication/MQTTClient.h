#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <../config.h>
#include <Arduino.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <TinyGSM.h>

#include <string>
#include <iostream>
#include <sstream> // for ostringstream
#include <vector>

#include <MessageSerializer.h>
#include <ReadingStore.h>

void taskMQTT(void * parameters);
void callbackMQTT(char* topic, byte* payload, unsigned int len);

class MQTTClient : private MessageSerializer
{
    public:
        MQTTClient(TinyGsmClient * client);
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

        static std::vector<std::string> * mqtt_rx_buffer;
    
    private:
        std::vector<std::string> mqtt_details;
        uint16_t broker_port = 0;

        TaskHandle_t handleMQTT;
        TinyGsmClient * gsm_client = nullptr;
        PubSubClient * mqtt_client = nullptr;

        void writeMQTTDetails();
        void fillMQTTDetails();

        void connectMQTT();

        friend void taskMQTT(void * parameters);
        friend class PubSubClient;
        friend void callbackMQTT(char* topic, byte* payload, unsigned int len);
        
};


#endif