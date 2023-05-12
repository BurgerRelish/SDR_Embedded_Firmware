#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>
#include <../config.h>
#include <Preferences.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include <string>
#include <iostream>
#include <sstream> // for ostringstream
#include <vector>

#include <../Communication/MessageSerializer.h>

void taskMQTT(void * parameters);
void callbackMQTT(char* topic, byte* payload, unsigned int len);

class MQTTClient : private MessageSerializer
{
    public:
        MQTTClient();
        explicit MQTTClient(Client * network_client);

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
        static std::vector<std::string> * mqtt_details;
        uint16_t broker_port = 0;

        static std::vector<std::string> * mqtt_rx_buffer;
        static std::vector<std::string> * mqtt_tx_buffer;
        static StaticSemaphore_t MQTT_rx_queue_buffer;
        static SemaphoreHandle_t MQTT_rx_queue_semaphore;
        static StaticSemaphore_t MQTT_tx_queue_buffer;
        static SemaphoreHandle_t MQTT_tx_queue_semaphore;

        static TaskHandle_t handleMQTT;
        static Client * network_client;
        static PubSubClient * mqtt_client;

        void initBuffers();

        void recallMQTTDetails();
        void writeMQTTDetails();
        void fillMQTTDetails();

        void connectMQTT();

        friend void taskMQTT(void * parameters);
        friend class PubSubClient;
        friend void callbackMQTT(char* topic, byte* payload, unsigned int len);
        
};


#endif