#include <Arduino.h>
#include <unity.h>
#include <ps_stl.h>
#include "ModuleInterface.h"

#define RS485_DIRECTION_PIN 11
#define CONTROL_LINE_1 9


/* EasyTransfer */
#define ADDRESS_PACKET_ID 0x10
struct AddressingPacket {
  uint16_t address;
  uint8_t operation;
} address_packet;

#define READING_PACKET_ID 0x20
struct ReadingDataPacket {
  uint8_t status;
  float voltage;
  float frequency;
  float current;
  float active_power;
  float power_factor;
  float energy_usage;
} reading_packet;

#define RELAY_BITMASK 0b00000001
#define READING_BITMASK 0b00000010
#define VOLTAGE_SAG_BITMASK 0b00000100
#define UFLS_LOCKOUT 0b00001000


void dynamicAddressing() {
    ModuleInterface interface(&Serial1, CONTROL_LINE_1, RS485_DIRECTION_PIN);
    
    auto found_modules = interface.begin();

    TEST_ASSERT_EQUAL(1, found_modules.size());
    
    auto module_1 = found_modules.at(0);

    TEST_ASSERT_EQUAL_STRING("a0297798-7d95-43dc-9aef-57ba606062f5", &module_1.first.id[0]);
    TEST_ASSERT_EQUAL(1, module_1.second); 
}


void setup() {
    pinMode(11, OUTPUT);
    pinMode(9, OUTPUT);

    Serial1.begin(9600, SERIAL_8N1, 18, 17);
    delay(2000);

    UNITY_BEGIN();
    RUN_TEST(dynamicAddressing);
    UNITY_END();
}

void loop() {

}
