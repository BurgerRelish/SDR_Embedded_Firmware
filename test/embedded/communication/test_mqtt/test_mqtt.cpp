#include <Arduino.h>
#include <unity.h>
#include <WiFi.h>
#include <FS.h>
#include <SPI.h>
#include <Wire.h>

#include "../lib/Serialization.h"

#define CLIENT_ID "1302c1a0-8244-4b18-be52-616a50527aec"
#define MQTT_ACCESS_TOKEN "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJ1c2VybmFtZSI6IjEzMDJjMWEwLTgyNDQtNGIxOC1iZTUyLTYxNmE1MDUyN2FlYyIsImV4cCI6MjAxMDc0NTk3NSwiYWNsIjp7InB1YiI6WyIvaW5ncmVzcyJdLCJzdWIiOlsiL2VncmVzcy8ke2NsaWVudGlkfSJdLCJhbGwiOltdfX0.30M0FyX0zd5V2merEU4aIZVY7GYr81UcH5NH6BfJTsM"
#define BROKER_URL "192.168.3.23"
#define BROKER_PORT 1883

WiFiClient wifi_client;

void test_mqtt() {
    auto client = ps::make_shared<MQTTClient>(wifi_client);

    client -> begin(CLIENT_ID, MQTT_ACCESS_TOKEN, BROKER_URL, BROKER_PORT);

    while (!client->incoming_message_count()) {}

    auto message = client -> get_incoming_message();
    ESP_LOGI("TEST", "Message Type: %s", message -> document["type"].as<const char*>());

    { // Message sent when class goes out of scope.
        auto new_message = client -> new_outgoing_message(0, 8192);
        new_message -> document = message -> document;
    }

    delay(2000); // Wait for message to send.
    
    TEST_ASSERT_TRUE(true);
}

void setup() {
    WiFi.begin("Routy", "0609660733");
    while (!WiFi.isConnected()){}


    UNITY_BEGIN();
    RUN_TEST(test_mqtt);
    UNITY_END();
    
}

void loop () {}