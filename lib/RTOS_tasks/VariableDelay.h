#pragma once

#ifndef VARIABLE_DELAY_H
#define VARIABLE_DELAY_H
#include "stdint.h"
#include <functional>
#include "esp32-hal-timer.h"

#include "../data_containers/ps_vector.h"
#include "../data_containers/ps_smart_ptr.h"

class callbackFN {
    private:
        const uint64_t target_interval;
        const std::function<void()> callback;
        uint64_t next_time;
        
    public:
        callbackFN(std::function<void()>& cb, uint32_t& target_ms) : callback(cb), target_interval(target_ms * 1000) {
            next_time = esp_timer_get_time();
        }

        void check() {
            auto current_tm = esp_timer_get_time();
            if (next_time < current_tm) {
                next_time += target_interval;
                callback();
                ESP_LOGD("CBFN", "[%u, %u]", (uint64_t)current_tm, next_time);
            }
        }
};

class  VariableDelay {
    private:
    std::vector<std::shared_ptr<callbackFN>> callbacks;

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