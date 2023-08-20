#include "InterfaceMaster.h"
#include <iostream>
#include <sstream>

uint8_t InterfaceMaster::get_end_device_address() {
    return end_address;
}

bool InterfaceMaster::assign_addresses() {
    SingleWire swi_interface(swi_pin);
    end_address = init_swi(&swi_interface);
    return end_address != -1;
}

int InterfaceMaster::init_swi(SingleWire* interface) {
    if (!interface->send(0, 100000)) return 0;
    if (!interface->await_receive(1500000)) return -1;
    return interface->receive();
}

bool InterfaceMaster::send(uint8_t device_address, ps::string message) {
    if (device_address > end_address) return false;

    digitalWrite(dir_pin, HIGH);
    auto ret = serial->printf("{%3.u|%s}", device_address, message.c_str());
    digitalWrite(dir_pin, LOW);

    return ret;
}

bool InterfaceMaster::send(uint8_t device_address, uint8_t command_number) {
    if (device_address > end_address) return false;

    digitalWrite(dir_pin, HIGH);
    auto ret = serial->printf("{%3.u|%3.u}", device_address, command_number);
    digitalWrite(dir_pin, LOW);

    return ret;
}

void InterfaceMaster::loop() {
    ps::ostringstream msg;
    while (serial->available()) {
        msg << static_cast<char>(serial->read());
    }

    auto payload = msg.str();
    if (payload.empty() || payload.front() != '[' || payload.back() != ']') return;

    payload.erase(payload.begin());
    payload.pop_back();
    receive_callback(payload);
}
