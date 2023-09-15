#include <functions/functions.h>
#include "sdr_semantics.h"

std::function<bool(ps::vector<ps::string>&, re::VariableStorage*)> publish_readings = [](ps::vector<ps::string>& args, re::VariableStorage* vars){
    bool new_state = vars->get_var<bool>(args.at(0));
    uint64_t change_time = vars->get_var<uint64_t>(CURRENT_TIME);

    vars -> set_var(re::VAR_BOOL, SWITCH_STATUS, new_state);
    vars -> set_var(re::VAR_UINT64_T, SWITCH_TIME, change_time);

    return true;
};

std::function<bool(ps::vector<ps::string>&, re::VariableStorage*)> notify = [](ps::vector<ps::string>& args, re::VariableStorage* vars){
    bool new_state = vars->get_var<bool>(args.at(0));
    uint64_t change_time = vars->get_var<uint64_t>(CURRENT_TIME);

    vars -> set_var(re::VAR_BOOL, SWITCH_STATUS, new_state);
    vars -> set_var(re::VAR_UINT64_T, SWITCH_TIME, change_time);

    return true;
};

std::function<bool(ps::vector<ps::string>&, re::VariableStorage*)> restart = [](ps::vector<ps::string>& args, re::VariableStorage* vars){
    esp_restart();
    return true;
};

std::function<bool(ps::vector<ps::string>&, re::VariableStorage*)> sleep_fn = [](ps::vector<ps::string>& args, re::VariableStorage* vars){
    uint64_t sleep_tm = vars -> get_var<uint64_t>(args.at(0));
    esp_sleep_enable_timer_wakeup(sleep_tm);
    esp_deep_sleep_start();
    return true;
};

void load_unit_functions(std::shared_ptr<re::FunctionStorage>& storage) {
    storage -> add(PUBLISH_READINGS, publish_readings);
    storage -> add(SLEEP_UNIT, sleep_fn);
    storage -> add(NOTIFY, notify);
    storage -> add(RESTART_UNIT, restart);
}