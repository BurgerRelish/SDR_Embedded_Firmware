#pragma once

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <PubSubClient.h>
#include <Client.h>

#include <functional>
#include <string>
#include "../Communication/MessageDeserializer.h"
#include "../data_containers/ps_string.h"
#include "../data_containers/ps_vector.h"

class MQTTClient {
    private:
        Client* connectivity_client;
        PubSubClient* mqtt_client = nullptr;
        std::function<void(MessageDeserializer*)> rx_callback;

        ps_string _server;
        uint32_t _port;
        ps_string _username;
        ps_string _password;
        ps_vector<ps_string> _topics;
        
        bool recallParams();
        bool ready();

        friend PubSubClient;
        void mqtt_callback(char* topic, uint8_t* payload, uint32_t length);

    public:
        /* Start an empty mqtt client, requires configuration using the setX commands;*/
        MQTTClient() {
            recallParams();
        }

        /* Create an MQTTClient with provided client and callback. Will call begin() if all parameters are initialised. */
        MQTTClient(Client* client, std::function<void(MessageDeserializer*)> callback) {
            recallParams();
            setCallback(callback);
            setConnectivityClient(client);

            if(ready()) begin();
        }

        /* Create an MQTTClient with the provided details. Will store client credentials to the NVS, and overwrite any existing values.*/
        MQTTClient(Client* client, std::function<void(MessageDeserializer*)> callback, ps_string& server, uint32_t& port, 
                    ps_string& username, ps_string& password,
                    ps_vector<ps_string>& topics) {
                setConnectivityClient(client);
                setCallback(callback);
                setServer(server, port);
                setCredentials(username, password);
                setTopics(topics);

                storeParams();
                if(ready()) begin();
            }

        ~MQTTClient() {
            if (mqtt_client != nullptr) {
                delete mqtt_client;
            }
        }

        void setConnectivityClient(Client* client) {connectivity_client = client;}
        void setCallback(std::function<void(MessageDeserializer*)> callback) {rx_callback = callback;}
        void setServer(ps_string& server, uint32_t& port) {
            _server = server;
            _port = port;
        }
        void setCredentials(ps_string& username, ps_string& password) {
            _username = username;
            _password = password;
        }
        void setTopics(ps_vector<ps_string>& topics) {_topics = topics;}
        ps_vector<ps_string>& getTopics() {return _topics;}

        bool storeParams();
        bool deleteParams();
        bool begin();

        PubSubClient* getClient() {
            return mqtt_client;
        }

        bool send(ps_string& message);
        
};

#endif