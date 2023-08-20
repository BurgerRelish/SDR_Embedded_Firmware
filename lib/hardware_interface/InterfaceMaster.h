#pragma once

#ifndef INTERFACE_MASTER_H
#define INTERFACE_MASTER_H

#include "stdint.h"
#include "SingleWire.h"
#include <ps_stl.h>
#include <functional>

class InterfaceMaster {
private:
    HardwareSerial* serial;
    uint8_t swi_pin;
    uint8_t dir_pin;
    uint8_t end_address;

    std::function<void(ps::string)> receive_callback;

    int init_swi(SingleWire* interface);

public:
    InterfaceMaster(HardwareSerial* serial_interface, uint8_t dir_pin, uint8_t swi_pin,
                    std::function<void(ps::string)> callback)
        : serial(serial_interface), swi_pin(swi_pin), dir_pin(dir_pin), receive_callback(callback) {
        pinMode(dir_pin, OUTPUT);
        digitalWrite(dir_pin, LOW);
    }

    uint8_t get_end_device_address();

    bool assign_addresses();
    bool send(uint8_t device_address, ps::string message);
    bool send(uint8_t device_address, uint8_t command_number);
    void loop();
};

#endif
