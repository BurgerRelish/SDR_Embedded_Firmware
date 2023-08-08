#include <Arduino.h>
#include <LittleFS.h>
#include <unity.h>

#include "../SDR/Persistence.h"

void test_write() {
    Persistence<fs::LittleFSFS> nvs(LittleFS, "/test.txt", 1024, true);
    nvs.document["test"].set("Hello World!!!");
    if (nvs.document["test"].as<std::string>() == "Hello World!!!") {
    TEST_ASSERT_TRUE(true);
    } else {
        TEST_ASSERT_TRUE(false);
    }
}

void test_read() {
    Persistence<fs::LittleFSFS> nvs(LittleFS, "/test.txt", 1024);
    std::string result = nvs.document["test"].as<std::string>();
    
    TEST_ASSERT_EQUAL_STRING("Hello World!!!", result.c_str());
}

void setup() {
    LittleFS.begin(true);
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_write);
    RUN_TEST(test_read);
    UNITY_END();
}

void loop() {

}