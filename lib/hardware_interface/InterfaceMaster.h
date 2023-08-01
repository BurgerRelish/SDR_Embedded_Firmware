#pragma once

#ifndef INTERFACE_MASTER_H
#define INTERFACE_MASTER_H

#include "stdint.h"
#include "SingleWireLL.h"
#include <string>
#include <functional>


class InterfaceMaster {
    private:
    HardwareSerial* serial_0;
    uint8_t swi_0;
    uint8_t end_address_0;

    HardwareSerial* serial_1;
    uint8_t swi_1;
    uint8_t end_address_1;

    std::function<void(uint8_t, std::string)> receive_callback;

    int init_swi(SingleWireLL* interface);

    public:
    InterfaceMaster(HardwareSerial* lhs_serial_interface, uint8_t lhs_swi_pin,
                    HardwareSerial* rhs_serial_interface, uint8_t rhs_swi_pin, 
                    std::function<void(uint8_t, std::string)> callback) 
    :   serial_0(lhs_serial_interface),
        swi_0(lhs_swi_pin),
        serial_1(rhs_serial_interface),
        swi_1(rhs_swi_pin),
        receive_callback(callback)
    {
    }
    
    uint8_t get_end_device_address(uint8_t interface);

    bool assign_addresses();
    bool send(uint8_t interface, uint8_t device_address, std::string message);
    bool send(uint8_t interface, uint8_t device_address, uint8_t command_number);
    void loop();
};

#endif