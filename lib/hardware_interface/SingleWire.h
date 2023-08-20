#pragma once

#ifndef SINGLE_WIRE_H
#define SINGLE_WIRE_H
#include <Arduino.h>

#define PERIOD_US 100 
#define MASTER_TIMEOUT_US 100000 // 100ms Timeout.

class SingleWire {
    private:
    gpio_num_t pin;
    volatile bool wait_receive;

    void attachISR();
    void ARDUINO_ISR_ATTR detachISR();

    public:
    SingleWire(gpio_num_t interface_pin);
    SingleWire(uint8_t interface_pin);

    bool send(uint8_t val, uint32_t timeout_us);
    int receive();
    bool await_receive(uint32_t timeout_us);

    void ARDUINO_ISR_ATTR receive_requested();
};

#endif