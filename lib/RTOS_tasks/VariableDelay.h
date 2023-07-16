#pragma once

#ifndef VARIABLE_DELAY_H
#define VARIABLE_DELAY_H
#include "stdint.h"

class  VariableDelay {
    private:
    uint64_t last_time = 0;
    uint32_t target_loop_time = 100;
    const char* tag_str;

    public:
    VariableDelay(const char* tag, const uint8_t target_frequency);

    void delay();
};

#endif