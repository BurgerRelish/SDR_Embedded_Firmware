#pragma once

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <PubSubClient.h>
#include <Client.h>
#include <WiFi.h>
#include <functional>
#include <memory>
#include <string>
#include <ps_stl.h>

#include "ps_base64.h"
#include "MessageSerializer.h"
#include "MessageDeserializer.h"

class MessageSerializer;
class MessageDeserializer;

class MQTTClient : public std::enable_shared_from_this<MQTTClient> {
    private:
        friend void communicationTask(void* parent);
        friend class MessageSerializer;
        friend class PubSubClient;

        TaskHandle_t task_handle;
        QueueHandle_t incoming_messages_queue;
        QueueHandle_t outgoing_messages_queue;
        
        Client& conn_client;
        
        ps::string client_id;
        ps::string auth_token;
        ps::string broker_url;
        uint16_t broker_port;

        void send_message(size_t topic, ps::string message);

    public:
        MQTTClient(Client& connection);

        ~MQTTClient();

        bool begin(const char* clientid, const char* token, const char* url, uint16_t port);

        std::shared_ptr<MessageSerializer> new_outgoing_message(size_t topic_number, size_t size);
        
        size_t incoming_message_count();
        std::shared_ptr<MessageDeserializer> get_incoming_message();
};

/**
 * @brief Creates a JSON document of requested size on construction. The JSON document can be modified during the lifetime of the class,
 * finally, when the class runs out of scope, any data in the JSON document is automatically serialized and sent using the MQTT Client to
 * the requested topic.
 */
class MessageSerializer {
    private:
        std::shared_ptr<MQTTClient> mqtt_client;
        size_t topic;
    public:
        DynamicPSRAMJsonDocument document;
        MessageSerializer(std::shared_ptr<MQTTClient> client, size_t topic_number,  size_t json_document_size);
        ~MessageSerializer();
};

/**
 * @brief Automatically deserializes the JSON formatted string. The JSON document can be used to get any data contained in the message. 
 * 
 */
class MessageDeserializer {
    private:
        ps::string message; // JSON document data must remain in memory whilst the document is in use.
    public:
        ps::string topic;
        DynamicPSRAMJsonDocument document;
        MessageDeserializer(const char* topic,const char* message);

};


#endif