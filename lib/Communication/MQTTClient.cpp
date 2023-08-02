#include "MQTTClient.h"
#include <PubSubClient.h>
#include <FS.h>
#include <LittleFS.h>

#include "json_allocator.h"


bool MQTTClient::storeParams(){
    JsonDoc document = JsonDoc(1024);

    document["username"].set(_username.c_str());
    document["password"].set(_password.c_str());
    document["server"].set(_server.c_str());
    document["port"].set(_port);
    auto topic_list = document.createNestedArray("topics");

    for (size_t i = 0; i < _topics.size(); i++) {
        topic_list.add(_topics.at(i).c_str());
    }

    ps_string output;
    serializeJson(document, output);
    
    LITTLEFS.begin(true);
    auto file = LITTLEFS.open("/mqtt/params.txt", "w", true);
    auto result = file.print(output.c_str());
    file.close();
    LITTLEFS.end();

    return (result == output.length());
}

bool MQTTClient::recallParams(){
    LITTLEFS.begin(true);
    auto file = LITTLEFS.open("/mqtt/params.txt", "r", false);
    if (!file) return false;

    ps_string input;
    while(file.available()) {
        input += (char) file.read();
    }

    file.close();
    LITTLEFS.end();

    JsonDoc document = JsonDoc(1024);
    if (deserializeJson(document, input).code() != 0) return false;

    _username.clear();
    _username <<= document["username"];
    _password <<= document["password"];
    _server <<= document["server"];
    _port = document["port"];

    auto array = document["topics"].as<JsonArray>();
    _topics.clear();
    for (JsonVariant v : array) {
        _topics.push_back(v.as<ps_string>());
    }

    return true;
}

bool MQTTClient::deleteParams() {
    LITTLEFS.begin(true);
    auto ret = LITTLEFS.remove("/mqtt/params.txt");
    LITTLEFS.end();

    return ret;
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
    if (mqtt_client != nullptr) {
        mqtt_client = new PubSubClient(_server.c_str(), _port, *connectivity_client);
    }

    mqtt_client->setCallback([this](char* topic, uint8_t* payload, uint32_t length) { this->mqtt_callback(topic, payload, length); });
    mqtt_client->connect(_username.c_str(), _username.c_str(), _password.c_str());

    for (size_t i = 1; i < _topics.size(); i++) {
        mqtt_client -> subscribe(_topics.at(i).c_str());
    }

    
    
}

void MQTTClient::mqtt_callback(char* topic, uint8_t* payload, uint32_t length) {
    ESP_LOGI("MQTT", "Got MQTT Message:");
    log_printf("Topic: %s\n", topic);
    ps_string message((char*) payload, length);
    log_printf("Message: %s\n", message.c_str());

    MessageDeserializer request(message);

    rx_callback(&request);

    return;
}

bool MQTTClient::send(ps_string& message) {
    if (mqtt_client == nullptr) return false;
    if (!mqtt_client -> connected()) return false;

    return mqtt_client -> publish(_topics.at(0).c_str(), message.c_str());
}