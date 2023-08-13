#include "RuleEngine.h"
#include <time.h>

namespace re {
    
void RuleEngine::load_rule_engine_vars() {
    variables -> set_var(VAR_UINT64_T, LAST_EXECUTION_TIME, std::function<uint64_t()>([this]() { return this->last_time; }));

    variables -> set_var(VAR_UINT64_T, CURRENT_TIME, std::function<uint64_t()>([]() {
        time_t now;
        struct tm timeinfo;
        if(!getLocalTime(&timeinfo)){
            ESP_LOGE("Time", "Failed to get time.");
            return static_cast<uint64_t>(0);
        }

        time(&now); 
        return static_cast<uint64_t>(now);
        }));
}

}