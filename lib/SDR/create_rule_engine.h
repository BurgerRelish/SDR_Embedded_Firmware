    // static std::shared_ptr<VariableStorage> load(std::shared_ptr<SDRUnit>&, std::shared_ptr<Module>&);
    // static void load_unit(std::shared_ptr<SDRUnit>&, std::shared_ptr<VariableStorage>&);
    // static void load_module(std::shared_ptr<Module>&, std::shared_ptr<VariableStorage>&);

    //     template <typename T>
    // void RuleEngine::load_rules(std::shared_ptr<T>& src);

// std::shared_ptr<VariableStorage> RuleEngine::load(std::shared_ptr<SDRUnit>& unit, std::shared_ptr<Module>& module) {
//     auto storage = ps::make_shared<VariableStorage>();
//     load_module(module, storage);
//     load_unit(unit, storage);

//     return storage;
// }

// void RuleEngine::load_unit(std::shared_ptr<SDRUnit>& unit, std::shared_ptr<VariableStorage>& storage) {
//     storage->set(VAR_CLASS, "UNIT", unit);

//     storage->set(VAR_DOUBLE, TOTAL_ACTIVE_POWER, std::function<double()>([unit]() { return unit->totalActivePower(); }));
//     storage->set(VAR_DOUBLE, TOTAL_REACTIVE_POWER, std::function<double()>([unit]() { return unit->totalReactivePower(); }));
//     storage->set(VAR_DOUBLE, TOTAL_APPARENT_POWER, std::function<double()>([unit]() { return unit->totalApparentPower(); }));
//     storage->set(VAR_BOOL, POWER_STATUS, std::function<bool()>([unit]() { return unit->powerStatus(); }));
//     storage->set(VAR_STRING, UNIT_ID, std::function<ps::string()>([unit]() { return unit->id(); }));
//     storage->set(VAR_ARRAY, UNIT_TAG_LIST, std::function<ps::vector<ps::string>()>([unit]() { return unit->getTags(); }));
//     storage->set(VAR_INT, MODULE_COUNT, std::function<int()>([unit]() { return unit->moduleCount(); }));
// }

// void RuleEngine::load_module(std::shared_ptr<Module>& module, std::shared_ptr<VariableStorage>& storage) {
//     storage->set(VAR_CLASS, "MODULE", module);

//     storage->set(VAR_DOUBLE, ACTIVE_POWER, std::function<double()>([module]() { return module->latestReading().active_power; }));
//     storage->set(VAR_DOUBLE, REACTIVE_POWER, std::function<double()>([module]() { return module->latestReading().reactive_power; }));
//     storage->set(VAR_DOUBLE, APPARENT_POWER, std::function<double()>([module]() { return module->latestReading().apparent_power; }));
//     storage->set(VAR_DOUBLE, VOLTAGE, std::function<double()>([module]() { return module->latestReading().voltage; }));
//     storage->set(VAR_DOUBLE, FREQUENCY, std::function<double()>([module]() { return module->latestReading().frequency; }));
//     storage->set(VAR_DOUBLE, POWER_FACTOR, std::function<double()>([module]() { return module->latestReading().power_factor; }));
//     storage->set(VAR_UINT64_T, SWITCH_TIME, std::function<uint64_t()>([module]() { return module->switchTime(); }));
//     storage->set(VAR_INT, CIRCUIT_PRIORITY, std::function<int()>([module]() { return module->priority(); }));
//     storage->set(VAR_STRING, MODULE_ID, std::function<ps::string()>([module]() { return module->id(); }));
//     storage->set(VAR_ARRAY, MODULE_TAG_LIST, std::function<ps::vector<ps::string>()>([module]() { return module->getTags(); }));
//     storage->set(VAR_BOOL, SWITCH_STATUS, std::function<bool()>([module]() { return module->status(); }));
// }


// template <typename T>
// void RuleEngine::load_rules(std::shared_ptr<T>& src) {
//     auto& rules = src -> getRules();
//     for (auto& rule : rules) {
//         add_rule(
//             rule.priority,
//             rule.expression,
//             rule.command
//         );
//     }
// }



// /* SDR Unit Variables */
// #define TOTAL_ACTIVE_POWER "TAP"
// #define TOTAL_REACTIVE_POWER "TRP"
// #define TOTAL_APPARENT_POWER "TSP"
// #define POWER_STATUS "PS"
// #define UNIT_ID "UID"
// #define UNIT_TAG_LIST "UTL"
// #define MODULE_COUNT "MC"


// /* Module Variables */
// #define ACTIVE_POWER "AP"
// #define REACTIVE_POWER "RP"
// #define APPARENT_POWER "SP"
// #define VOLTAGE "V"
// #define FREQUENCY "FR"
// #define POWER_FACTOR "PF"
// #define SWITCH_TIME "SWT"
// #define CIRCUIT_PRIORITY "CP"
// #define MODULE_ID "MID"
// #define MODULE_TAG_LIST "MTL"
// #define SWITCH_STATUS "MS"

// /* Commands */
// #define MOD_ON "ON" // ()
// #define MOD_OFF "OFF" // ()

// #define RE_CLR_QUEUE "CLRQUE" // ()
// #define RE_DELAY "DELAY" // ()

// #define REQUEST_UPDATE "REQUPD" // ()
// #define PUBLISH_READINGS "PUBREAD" // ()
// #define NOTIFY "NOTIFY" // ()

// #define RESTART_UNIT "RESTART" // ()