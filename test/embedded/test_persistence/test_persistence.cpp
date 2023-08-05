#include <Arduino.h>
#include <LittleFS.h>
#include <unity.h>

#include "../Persistence.h"

void test_write() {
    Persistence nvs(LittleFS, "test.txt");
    nvs.document["ssid"].set("Hello World!!!");
    TEST_ASSERT_TRUE(true);
}

void test_read() {
    Persistence nvs(LittleFS, "test.txt");
    TEST_ASSERT_EQUAL_STRING("Hello World!!!", nvs.document["ssid"].as<String>().c_str());
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_write);
    RUN_TEST(test_read);
    UNITY_END();
}

void loop() {

}