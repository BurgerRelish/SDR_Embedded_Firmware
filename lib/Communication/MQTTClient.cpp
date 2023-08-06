#include "MQTTClient.h"
#include <PubSubClient.h>
#include <FS.h>
#include <LittleFS.h>

#include "json_allocator.h"
#include "../data_containers/ps_smart_ptr.h"


bool MQTTClient::storeParams(){
    return true;
}

bool MQTTClient::recallParams(){

    return true;
}

bool MQTTClient::deleteParams() {

    return true;
}

bool MQTTClient::ready() {
    bool ret = (_username.length() > 0);
    ret = ret && (_password.length() > 0);
    ret = ret && (_server.length() > 0);
    ret = ret && (_port > 0);
    ret = ret && (_topics.size() > 0);

    return ret;
}


bool MQTTClient::begin() {
    if (!ready()) return false;

    mqtt_client.setCallback([this](char* topic, uint8_t* payload, uint32_t length) { this->mqtt_callback(topic, payload, length); });
    mqtt_client.connect(_username.c_str(), _username.c_str(), _password.c_str());

    for (size_t i = 1; i < _topics.size(); i++) {
        mqtt_client.subscribe(_topics.at(i).c_str());
    }

    return true;
}

void MQTTClient::mqtt_callback(char* topic, uint8_t* payload, uint32_t length) {
    ESP_LOGI("MQTT", "Got MQTT Message:");
    log_printf("Topic: %s\n", topic);
    ps_string message((char*) payload, length);
    log_printf("Message: %s\n", message.c_str());

    rx_callback(ps::make_shared<MessageDeserializer>(message));

    return;
}

bool MQTTClient::send(ps_string& message) {
    if (!mqtt_client.connected()) return false;

    return mqtt_client.publish(_topics.at(0).c_str(), message.c_str());
}