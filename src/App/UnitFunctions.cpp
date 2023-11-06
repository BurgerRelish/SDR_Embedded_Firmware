#include "UnitFunctions.h"
#include "SDRSemantics.h"
#include "Unit.h"

/**
 * @brief Publish all readings.
 * 
 */
std::function<bool(ps::vector<ps::vector<Token>>&, re::VariableStorage*)> publish_readings = [](ps::vector<ps::vector<Token>>& args, re::VariableStorage* vars){
    Unit* unit = (Unit*) vars -> get_var<void*>(UNIT_CLASS);
    unit -> publishReadings();
    return true;
};


/**
 * @brief Read all modules and refresh unit variables.
 * 
 */
std::function<bool(ps::vector<ps::vector<Token>>&, re::VariableStorage*)> refresh_unit = [](ps::vector<ps::vector<Token>>& args, re::VariableStorage* vars){
    Unit* unit = (Unit*) vars -> get_var<void*>(UNIT_CLASS);
    return unit -> refresh();
};


/**
 * @brief Delay the unit for some time.
 * 
 * @arg arg[0] - `uint64_t` - Delay time in milliseconds.
 */
std::function<bool(ps::vector<ps::vector<Token>>&, re::VariableStorage*)> delay_fn = [](ps::vector<ps::vector<Token>>& args, re::VariableStorage* vars){
    re::Expression expr(args.at(0), vars);
    double delay_ms = expr.result();
    vTaskDelay(delay_ms / portTICK_PERIOD_MS);

    return true;
};

std::function<bool(ps::vector<ps::vector<Token>>&, re::VariableStorage*)> restart = [](ps::vector<ps::vector<Token>>& args, re::VariableStorage* vars){
    esp_restart();
    return true;
};

/**
 * @brief Put the ESP to sleep for some time.
 * @note ESP will restart when it wakes up.
 * 
 * @arg uint64_t - Sleep time in milliseconds.
 */
std::function<bool(ps::vector<ps::vector<Token>>&, re::VariableStorage*)> sleep_fn = [](ps::vector<ps::vector<Token>>& args, re::VariableStorage* vars){
    re::Expression expr(args.at(0), vars);
    double sleep_tm = expr.result();
    esp_sleep_enable_timer_wakeup(sleep_tm * 1000);
    esp_deep_sleep_start();
    return true;
};

void load_unit_functions(std::shared_ptr<re::FunctionStorage>& storage) {
    storage -> add(PUBLISH_READINGS, publish_readings);
    storage -> add(SLEEP_UNIT, sleep_fn);
    storage -> add(RESTART_UNIT, restart);
    storage -> add(DELAY_UNIT, delay_fn);
    storage -> add(READ_MODULES, refresh_unit);
    storage -> add(PUBLISH_READINGS, publish_readings);
}