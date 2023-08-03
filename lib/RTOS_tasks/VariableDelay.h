#pragma once

#ifndef VARIABLE_DELAY_H
#define VARIABLE_DELAY_H
#include "stdint.h"
#include <functional>
#include "../data_containers/ps_vector.h"

class  VariableDelay {
    private:

    struct callbackFN {
        uint32_t target_interval;
        uint64_t last_time;
        std::function<void()> callback;
    };

    ps_vector<callbackFN> callbacks;

    uint64_t last_time = 0;
    uint32_t target_loop_time = 100;
    const char* tag_str;

    public:
    VariableDelay(const char* tag, const uint8_t target_frequency);

    void addCallback(std::function<void()> callback_fn, uint32_t target_interval_ms);
    void loop();
    void delay();
};

#endif