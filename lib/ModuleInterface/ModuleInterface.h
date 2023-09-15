#pragma once

#ifndef EASY_INTERFACE_H
#define EASY_INTERFACE_H
#include <Arduino.h>
#include <ps_stl.h>
#include "./EasyTransfer/src/EasyTransfer.h"

struct AnnouncePacket {
    char id[38];
    uint16_t firmware_version;
    uint16_t hardware_version;
};

struct OperationPacket {
    uint16_t operation;
};

struct AddressPacket {
    uint16_t address;
};

struct ReadingDataPacket {
  uint8_t status;
  float voltage;
  float frequency;
  float current;
  float apparent_power;
  float power_factor;
  float energy_usage;
};


#define OPERATION_RELAY_SET 0x0001
#define OPERATION_RELAY_RESET 0x0002
#define OPERATION_READ_METER 0x0004

class ModuleInterface {
    public:
    ModuleInterface(Stream* serial, uint8_t control_line, uint8_t dir_pin);
    
    ps::vector<std::pair<AnnouncePacket, uint8_t>> begin();

    bool sendOperation(uint8_t address, uint16_t operation);
    ReadingDataPacket getReading(uint8_t address);

    private:
    Stream* stream;
    uint8_t ctrl;
    uint8_t dir;

    uint8_t current_address;

    friend class EasyTransfer;
    EasyTransfer transfer_in;
    EasyTransfer transfer_out;

    OperationPacket operation_packet;
    ReadingDataPacket reading_packet;

    void clearStreamBuffer();
    bool transmit(uint8_t address);
    bool receive();

};

#endif