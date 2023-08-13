#include "VariableDelay.h"

#include "freertos/task.h"
#include "cstring"

VariableDelay::VariableDelay(const char* tag, const uint8_t target_frequency) {
    target_loop_time = 1000 / target_frequency;
    last_time = esp_timer_get_time();
    tag_str = tag;
}

void VariableDelay::delay() {
    uint32_t elapsed_time = ((esp_timer_get_time() - last_time) / 1000);

    if(elapsed_time > target_loop_time) {
        last_time = esp_timer_get_time();
        ESP_LOGE(tag_str, "Task running slower than target frequency. [%.2fHz]", (1000 / ((float) elapsed_time)));
        return;
    }

    vTaskDelay((target_loop_time - elapsed_time) / portTICK_PERIOD_MS);
    last_time = esp_timer_get_time();
    
    return;
}

void VariableDelay::addCallback(std::function<void()> callback_fn, uint32_t target_interval_ms) {
    callbacks.push_back(
        ps::make_shared<callbackFN> (
            callback_fn,
            target_interval_ms
        )
    );
    return;
}

void VariableDelay::loop() {
    for(auto& callback : callbacks) {
        callback -> check();
    }
    delay();
}