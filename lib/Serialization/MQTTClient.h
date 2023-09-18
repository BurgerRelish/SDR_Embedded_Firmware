#pragma once

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <PubSubClient.h>
#include <Client.h>
#include <WiFi.h>

#include <functional>
#include <memory>
#include <string>
#include "MessageDeserializer.h"
#include <ps_stl.h>


enum ConnectivityMode {
    CONN_WIFI,
    CONN_GSM
};

class MQTTClient {
    private:
        
        Client& conn_client;
        PubSubClient mqtt_client;

        std::function<void(std::shared_ptr<MessageDeserializer>)> rx_callback;


        ps::string _server;
        uint32_t _port;
        ps::string _username;
        ps::string _password;
        ps::vector<ps::string> _egress;
        ps::vector<ps::string> _ingress;

        bool ready();

        friend PubSubClient;
        void mqtt_callback(char* topic, uint8_t* payload, uint32_t length);

    public:
        /* Create an MQTTClient with the provided details. Will store client credentials to the NVS, and overwrite any existing values.*/
        MQTTClient(Client& connectivity_client, 
                    std::function<void(std::shared_ptr<MessageDeserializer>)> callback,
                    ps::string server,
                    uint32_t port, 
                    ps::string username,
                    ps::string password,
                    ps::vector<ps::string> ingress_topics,
                    ps::vector<ps::string> egress_topics
                ) : conn_client(connectivity_client)
            {
                setServer(server, port);
                setCredentials(username, password);
                setTopics(ingress_topics, egress_topics);
                if(ready()) begin();
            }

        ~MQTTClient() {
        }

        void setServer(ps::string& server, uint32_t& port) {
            _server = server;
            _port = port;
        }

        void setCredentials(ps::string& username, ps::string& password) {
            _username = username;
            _password = password;
        }

        void setTopics(ps::vector<ps::string>& ingress_topics, ps::vector<ps::string>& egress_topics) {
            _egress = egress_topics;
            _ingress = ingress_topics;
        }

        ps::vector<ps::string>& getIngressTopics() {return _ingress;}
        ps::vector<ps::string>& getEgressTopics() {return _egress;}

        bool begin();

        PubSubClient& getClient() {
            return mqtt_client;
        }

        bool send(ps::string& message, uint16_t egress_topic_number);
        bool send(ps::string& message, ps::string& topic);
        
};

#endif