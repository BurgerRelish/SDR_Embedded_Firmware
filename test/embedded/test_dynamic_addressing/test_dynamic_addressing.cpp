#include <Arduino.h>
#include <unity.h>
#include <ps_stl.h>
#include "ModuleInterface.h"
#include <FS.h>
#include <WiFi.h>
#include <SPI.h>
#include <Wire.h>

#define RS485_DIRECTION_PIN 11
#define CONTROL_LINE_1 9

void dynamicAddressing() {
    ModuleInterface interface(&Serial1, CONTROL_LINE_1, RS485_DIRECTION_PIN);
    
    auto found_modules = interface.begin();

    TEST_ASSERT_EQUAL(1, found_modules.size());
    
    auto module_1 = found_modules.at(0);

    interface.sendOperation(1, OPERATION_RELAY_SET);
    delay(50);
    auto data = interface.getReading(1);
    TEST_ASSERT_EQUAL(1, data.status);
    delay(2500);
    interface.sendOperation(1, OPERATION_RELAY_RESET);
    delay(50);
    data = interface.getReading(1);
    TEST_ASSERT_EQUAL(0, data.status);

    TEST_ASSERT_EQUAL_STRING("a0297798-7d95-43dc-9aef-57ba606062f5", &module_1.first.id[0]);
    TEST_ASSERT_EQUAL(1, module_1.second); 
}


void setup() {
    pinMode(11, OUTPUT);
    pinMode(9, OUTPUT);
    digitalWrite(9, HIGH);
    Serial1.begin(115200, SERIAL_8N1, 18, 17);
    delay(2000);

    UNITY_BEGIN();
    RUN_TEST(dynamicAddressing);
    UNITY_END();
}

void loop() {

}
