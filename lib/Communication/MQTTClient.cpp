#include "MQTTClient.h"
#include <PubSubClient.h>

#include "json_allocator.h"


/**
 * @brief Checks whether the minimum required parameters are set for MQTT connection.
 * 
 * @return true - All required parameters are set.
 * @return false - Not all required parameters are set.
 */
bool MQTTClient::ready() {
    return (_username.length() > 0)
    && (_password.length() > 0)
    && (_server.length() > 0)
    && (_port > 0)
    && (_egress.size() > 0)
    && (_ingress.size() > 0);
}

/**
 * @brief If all parameters are ready, connects to the mqtt server and subscribes to all the provided ingress topics.
 * 
 * @return true 
 * @return false 
 */
bool MQTTClient::begin() {
    if (!ready()) return false; 
    
    mqtt_client.setClient(conn_client);
    mqtt_client.setServer(_server.c_str(), _port);
    mqtt_client.setCallback([this](char* topic, uint8_t* payload, uint32_t length) { this->mqtt_callback(topic, payload, length); });
    mqtt_client.connect(_username.c_str(), _username.c_str(), _password.c_str());
    
    for (size_t i = 1; i < _ingress.size(); i++) {
        mqtt_client.subscribe(_ingress.at(i).c_str());
    }

    return true;
}

void MQTTClient::mqtt_callback(char* topic, uint8_t* payload, uint32_t length) {
    ESP_LOGI("MQTT", "Got MQTT Message:");
    log_printf("Topic: %s\n", topic);
    ps::string message((char*) payload, length);
    log_printf("Message: %s\n", message.c_str());

    rx_callback(ps::make_shared<MessageDeserializer>(message));

    return;
}

/**
 * @brief Publishes a message to the first topic in the topic list.
 * 
 * @param message Message to publish.
 * @param egress_topic_number Number of topic in egress topic vector to publish to.
 * @return true - Successfully published.
 * @return false - Failed to publish.
 */
bool MQTTClient::send(ps::string& message, uint16_t egress_topic_number) {
    if (!mqtt_client.connected()) return false;

    return mqtt_client.publish(_egress.at(egress_topic_number).c_str(), message.c_str());
}

/**
 * @brief Publishes a message to the provided topic.
 * 
 * @param message Message to publish.
 * @param topic Topic to publish to.
 * @return true - Successfully published.
 * @return false - Failed to publish.
 */
bool MQTTClient::send(ps::string& message, ps::string& topic) {
    if (!mqtt_client.connected()) return false;

    return mqtt_client.publish(topic.c_str(), message.c_str());
}

