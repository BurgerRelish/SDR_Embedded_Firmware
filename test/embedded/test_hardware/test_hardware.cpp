#include <Arduino.h>
#include <unity.h>
#include <ps_stl.h>
#include <ModbusRTUMaster.h>
#include "EasyTransfer.h"

#define RS485_DIRECTION_PIN 11

ModbusRTUMaster modbus(Serial1, RS485_DIRECTION_PIN);


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

EasyTransfer ETIn;
EasyTransfer ETOut;

void preTransmission()
{
    // Set the RS485 direction to transmit
    digitalWrite(RS485_DIRECTION_PIN, HIGH);
    delayMicroseconds(10);
}

void postTransmission()
{
    // Wait for the transmission to complete
    Serial1.flush();
    // Set the RS485 direction back to receive
    digitalWrite(RS485_DIRECTION_PIN, LOW);
}

void transmitRS485(const char* message) {
    preTransmission();
    
    // Send the message over Serial1
    Serial1.print(message);
    
    postTransmission();
}

bool dynamicAddressing() {
    
    while (Serial1.available() > 0) {
        char _ = Serial1.read(); // Read and discard the data
    }

    digitalWrite(9, LOW);
    delay(5);
    digitalWrite(9, HIGH);

    String str;

    while(!Serial1.available()) {}
    str = Serial1.readString();
    
    if (str == "[req:a0297798-7d95-43dc-9aef-57ba606062f5:0000:0000]") transmitRS485("{001}");
    else return false;

    while(!Serial1.available()) {}
    str = Serial1.readString();

    if (str == "[ack]") return true;
    return false;
}

void test_serial() {
    transmitRS485("Master\n");

    while(!dynamicAddressing()) {
    };

    transmitRS485("{mig}");
    delay(20); // Allow slaves to migrate.

    ETIn.begin(details(reading_packet),&Serial1, 0, RS485_DIRECTION_PIN);
    ETOut.begin(details(address_packet), &Serial1, 0, RS485_DIRECTION_PIN);

    while (1) {
        address_packet.address = 1;
        address_packet.operation = RELAY_BITMASK;


        while(!ETOut.sendData(1)) {
            delay(10);
        }

        delay(2500);
        address_packet.operation = 0;

        while(!ETOut.sendData(1)) {
            delay(10);
        }

        delay(2500);
    }

    TEST_ASSERT_TRUE(true);

}

void setup() {
    pinMode(11, OUTPUT);
    pinMode(9, OUTPUT);

    Serial1.begin(9600, SERIAL_8N1, 18, 17);
    delay(2000);

    UNITY_BEGIN();
    RUN_TEST(test_serial);
    UNITY_END();
}

void loop() {

}
