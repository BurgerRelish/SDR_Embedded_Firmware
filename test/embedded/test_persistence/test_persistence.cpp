#include <Arduino.h>
#include <unity.h>
#include <WiFi.h>
#include <FS.h>
#include <SPI.h>
#include <Wire.h>
#include <LittleFS.h>
#include "Persistence.h"

void test_write() {
    Persistence nvs("/test.txt", 1024, true);
    nvs.document["test"].set("Hello World!!!");
    if (nvs.document["test"].as<std::string>() == "Hello World!!!") {
    TEST_ASSERT_TRUE(true);
    } else {
        TEST_ASSERT_TRUE(false);
    }
}

void test_read() {
    Persistence nvs("/test.txt", 1024);
    std::string result = nvs.document["test"].as<std::string>();
    
    TEST_ASSERT_EQUAL_STRING("Hello World!!!", result.c_str());
}

void setup() {
    if (!LittleFS.begin(true)) ESP_LOGE("FS", "Failed to start.");
    auto file = LittleFS.open("/test1.txt", FILE_READ, true);
    ESP_LOGI("FS", "Got: %s", file.readString().c_str());
    file.close();

    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_write);
    RUN_TEST(test_read);
    UNITY_END();
}

void loop() {

}