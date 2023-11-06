#include "Functions.h"
#include "SDRSemantics.h"
#include "ModuleFunctions.h"
#include "UnitFunctions.h"



std::shared_ptr<re::FunctionStorage> load_functions() {
    auto fn = ps::make_shared<re::FunctionStorage>();
    load_unit_functions(fn);
    load_module_functions(fn);

    return fn;
}
