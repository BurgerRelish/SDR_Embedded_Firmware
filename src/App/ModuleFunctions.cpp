#include "ModuleFunctions.h"
#include "SDRSemantics.h"
#include "Module.h"


std::function<bool(ps::vector<ps::vector<Token>>&, re::VariableStorage*)> set_module_state = [](ps::vector<ps::vector<Token>>& args, re::VariableStorage* vars){
    re::Expression expr(args.at(0), vars);
    bool new_state = expr.evaluate();

    Module* module = (Module*) vars -> get_var<void*>(MODULE_CLASS);
    ESP_LOGI("ModuleCmd", "Setting state to: %d", new_state);
    return module -> setRelayState(new_state);
};

void load_module_functions(std::shared_ptr<re::FunctionStorage>& storage) {
    storage -> add(SET_MODULE_STATE, set_module_state);
}