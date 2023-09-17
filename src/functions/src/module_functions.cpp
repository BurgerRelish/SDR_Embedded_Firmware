#include <functions/functions.h>
#include "sdr_semantics.h"
#include "Module.h"

std::function<bool(ps::vector<ps::string>&, re::VariableStorage*)> set_module_state = [](ps::vector<ps::string>& args, re::VariableStorage* vars){
    bool new_state = vars->get_var<bool>(args.at(0));

    Module* module = (Module*) vars -> get_var<void*>(MODULE_CLASS);
    return module -> setRelayState(new_state);
};

void load_module_functions(std::shared_ptr<re::FunctionStorage>& storage) {
    storage -> add(SET_MODULE_STATE, set_module_state);
}