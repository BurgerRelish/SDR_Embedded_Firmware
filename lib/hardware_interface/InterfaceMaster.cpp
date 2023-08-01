#include "InterfaceMaster.h"
#include <iostream>
#include <sstream>

uint8_t InterfaceMaster::get_end_device_address(uint8_t interface) {
    return (interface) ? end_address_1 : end_address_0;
}

bool InterfaceMaster::assign_addresses() {
    SingleWireLL swi_interface_0(swi_0);
    SingleWireLL swi_interface_1(swi_1);

    end_address_0 = init_swi(&swi_interface_0);
    end_address_1 = init_swi(&swi_interface_1);

    if (end_address_0 == -1 || end_address_1 == -1) return false;
    return true;
}

int InterfaceMaster::init_swi(SingleWireLL* interface) {
    if (!interface -> send(0, 100000)) return 0; // The slave will recieve 0, increment it and store the value as its own address, then send its address to the next slave.
    if (!interface -> awaitReceive(1500000)) return -1; // Slave MCUs taking too long to assign addresses, abort
    return interface -> receive(); // The last slave will fail to pass its address on, thus will pass its address back along the line to the master.
}

bool InterfaceMaster::send(uint8_t interface, uint8_t device_address, std::string message) {
    auto serial = (interface) ? serial_1 : serial_0;

    if (device_address > (interface) ? end_address_1 : end_address_0) return false;

    auto ret = serial -> printf("{%3.u|%s}", device_address, message.c_str());
    
    return ret;
}

bool InterfaceMaster::send(uint8_t interface, uint8_t device_address, uint8_t command_number) {
    auto serial = (interface) ? serial_1 : serial_0;

    if (device_address > (interface) ? end_address_1 : end_address_0) return false;

    auto ret = serial -> printf("{%3.u|%3.u}", device_address, command_number);
    
    return ret;
}

void InterfaceMaster::loop() {
    for (uint8_t i = 0; i <= 1; i++) {
        auto serial = (i) ? serial_1 : serial_0;
        std::ostringstream msg;
        while (serial -> available()) {
            msg << (char)serial->read();
        }
        
        auto payload = msg.str();

        /* Confirm and then erase the start and end brackets */
        if (payload.at(0) != '[' || payload.at(payload.size() - 1) != ']') return;
        payload.erase(payload.begin());
        payload.erase(--payload.end());

        receive_callback(i, payload);
    }

    return;
}