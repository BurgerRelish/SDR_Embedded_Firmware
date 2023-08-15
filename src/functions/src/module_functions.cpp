#include <functions/functions.h>
#include "sdr_semantics.h"

std::function<bool(ps::vector<ps::string>&, re::VariableStorage*)> set_module_state = [](ps::vector<ps::string>& args, re::VariableStorage* vars){
    bool new_state = vars->get_var<bool>(args.at(0));
    uint64_t change_time = vars->get_var<uint64_t>(CURRENT_TIME);

    vars -> set_var(re::VAR_BOOL, SWITCH_STATUS, new_state);
    vars -> set_var(re::VAR_UINT64_T, SWITCH_TIME, change_time);

    return true;
};

void load_module_functions(std::shared_ptr<re::FunctionStorage>& storage) {
    storage -> add(SET_MODULE_STATE, set_module_state);
}