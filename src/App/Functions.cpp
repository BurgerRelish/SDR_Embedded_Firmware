#include "Functions.h"
#include "SDRSemantics.h"
#include "ModuleFunctions.h"
#include "UnitFunctions.h"

std::function<bool(ps::vector<ps::string>&, re::VariableStorage*)> set_variable = [](ps::vector<ps::string>& args, re::VariableStorage* vars){
    ESP_LOGI("SetVar", "Set %s, of type %s, to: %s", args.at(1).c_str(), args.at(0).c_str(), args.at(2).c_str());
    if (args.at(0) == "str") {
        vars -> set_var(re::VAR_STRING, args.at(1), vars -> get_var<ps::string>(args.at(2)));
        return true;
    } else if (args.at(0) == "int") {
        vars -> set_var(re::VAR_UINT64_T, args.at(1), vars -> get_var<uint64_t>(args.at(2)));
        return true;
    } else if (args.at(0) == "double") {
        vars -> set_var(re::VAR_DOUBLE, args.at(1), vars -> get_var<double>(args.at(2)));
        return true;
    } else if (args.at(0) == "bool") {
        vars -> set_var(re::VAR_BOOL, args.at(1), vars -> get_var<bool>(args.at(2)));
        return true;
    } else if (args.at(0) == "int") {
        vars -> set_var(re::VAR_INT, args.at(1), vars -> get_var<int>(args.at(2)));
        return true;
    }
    
    return false;
};

std::shared_ptr<re::FunctionStorage> load_functions() {
    auto fn = ps::make_shared<re::FunctionStorage>();
    load_unit_functions(fn);
    load_module_functions(fn);
    fn -> add(SET_VARIABLE, set_variable);

    return fn;
}
