#include "Unit.h"
#include <time.h>

void Unit::load_vars() {
    re::RuleEngineBase::set_var(re::VAR_CLASS, UNIT_CLASS, (void*)this);

    re::RuleEngineBase::set_var(re::VAR_DOUBLE, TOTAL_ACTIVE_POWER, std::function<double()>([this]() { return this->totalActivePower(); }));
    re::RuleEngineBase::set_var(re::VAR_DOUBLE, TOTAL_REACTIVE_POWER, std::function<double()>([this]() { return this->totalReactivePower(); }));
    re::RuleEngineBase::set_var(re::VAR_DOUBLE, TOTAL_APPARENT_POWER, std::function<double()>([this]() { return this->totalApparentPower(); }));
    re::RuleEngineBase::set_var(re::VAR_DOUBLE, MEAN_VOLTAGE, std::function<double()>([this]() { return this->meanVoltage(); }));
    re::RuleEngineBase::set_var(re::VAR_DOUBLE, MEAN_POWER_FACTOR, std::function<double()>([this]() { return this->meanPowerFactor(); }));
    re::RuleEngineBase::set_var(re::VAR_DOUBLE, MEAN_FREQUENCY, std::function<double()>([this]() { return this->meanFrequency(); }));

    re::RuleEngineBase::set_var(re::VAR_BOOL, POWER_STATUS, std::function<bool()>([this]() { return this->powerStatus(); }));
    re::RuleEngineBase::set_var(re::VAR_STRING, UNIT_ID, std::function<ps::string()>([this]() { return this->id(); }));
    re::RuleEngineBase::set_var(re::VAR_ARRAY, UNIT_TAG_LIST, std::function<ps::vector<ps::string>()>([this]() { return this->get_tags(); }));
    re::RuleEngineBase::set_var(re::VAR_INT, MODULE_COUNT, std::function<int()>([this]() { return this->moduleCount(); }));
}

void Unit::loadUnitVarsInModule(std::shared_ptr<Module>& module) {
    module -> set_var(re::VAR_DOUBLE, TOTAL_ACTIVE_POWER, std::function<double()>([this]() { return this->totalActivePower(); }));
    module -> set_var(re::VAR_DOUBLE, TOTAL_REACTIVE_POWER, std::function<double()>([this]() { return this->totalReactivePower(); }));
    module -> set_var(re::VAR_DOUBLE, TOTAL_APPARENT_POWER, std::function<double()>([this]() { return this->totalApparentPower(); }));
    module -> set_var(re::VAR_DOUBLE, MEAN_VOLTAGE, std::function<double()>([this]() { return this->meanVoltage(); }));
    module -> set_var(re::VAR_DOUBLE, MEAN_POWER_FACTOR, std::function<double()>([this]() { return this->meanPowerFactor(); }));
    module -> set_var(re::VAR_DOUBLE, MEAN_FREQUENCY, std::function<double()>([this]() { return this->meanFrequency(); }));

    module -> set_var(re::VAR_BOOL, POWER_STATUS, std::function<bool()>([this]() { return this->powerStatus(); }));
    module -> set_var(re::VAR_STRING, UNIT_ID, std::function<ps::string()>([this]() { return this->id(); }));
    module -> set_var(re::VAR_ARRAY, UNIT_TAG_LIST, std::function<ps::vector<ps::string>()>([this]() { return this->get_tags(); }));
    module -> set_var(re::VAR_INT, MODULE_COUNT, std::function<int()>([this]() { return this->moduleCount(); }));  
}

void Unit::begin(Stream* stream_1, uint8_t ctrl_1, uint8_t ctrl_2, uint8_t dir_1) {
    number_of_modules = 0;
    interface_1 = ps::make_shared<ModuleInterface>(stream_1, ctrl_1, ctrl_2, dir_1);
    auto found_modules = interface_1 -> begin();

    for (auto found_module : found_modules) {
        ESP_LOGI("Unit", "Found: %s", &found_module.first.id[0]);
        module_list.push_back(
            ps::make_shared<Module>(functions, interface_1, found_module.second, ps::string(found_module.first.id), found_module.first.firmware_version, found_module.first.hardware_version)
        );
        number_of_modules++;
    }

    create_module_map(); 
    last_serialization = getTime();
}

bool Unit::evaluateAll() {
    try {
        RuleEngineBase::reason();
        vTaskDelay(1 / portTICK_PERIOD_MS);
        evaluateModules();
    } catch (...) {
        ESP_LOGE("RULE_ENGINE", "Something went wrong evaluating unit.");
        return false;
    }

    return evaluateModules();
}

bool Unit::evaluateModules() {
    try {
        for (auto module : module_list) {
            module -> reason();
            vTaskDelay(1 / portTICK_PERIOD_MS);
        }
    } catch (...) {
        ESP_LOGE("RULE_ENGINE", "Something went wrong evaluating modules.");
        return false;
    }

    return true;
}

/**
 * @brief Create a ps::unordered_map with the discovered modules to allow for quick ID lookup.
 * 
 */
void Unit::create_module_map() {
    module_map = ps::unordered_map<ps::string, std::shared_ptr<Module>>();
    auto modules = getModules();

    for (auto module : modules) {
      module_map.insert( std::make_pair( module -> getModuleID(), module ));
    }
}

/**
 * @brief Calculate the unit class variables, read power status pin, read from all modules, and reason on their data. Finally, count how many modules are on.
 * Finally, evaluates the unit rule engine.
 * 
 * @return true 
 * @return false 
 */
bool Unit::refresh() {
    total_apparent_power = 0;
    total_active_power = 0;
    total_reactive_power = 0;
    mean_pf = 0;
    mean_voltage = 0;
    mean_frequency = 0;
    mean_current = 0;

    if (module_list.size() == 0) return false;

    for (auto module : module_list) {
        auto reading = module -> getLatestReading();
        total_apparent_power += reading.apparent_power;
        total_reactive_power += reading.reactive_power();
        total_active_power += reading.active_power();

        if (module -> getRelayState()) mean_pf += reading.power_factor / active_modules;
        mean_voltage += reading.voltage / number_of_modules;
        mean_frequency += reading.frequency / number_of_modules;
        mean_current += reading.current / number_of_modules;
    }

    active_modules = activeModules(); // Check which modules are on after reasoning.

    power_status = (analogRead(power_sense_pin) > 1000);

    return true;
}

bool Unit::load(JsonObject& obj) {
    auto rule_obj = obj["rule_engine"].as<JsonObject>();
    RuleEngineBase::load_rule_engine(rule_obj);
    return true;
}

bool Unit::save(JsonObject& obj) {
    auto rule_obj = obj.createNestedObject("rule_engine");
    RuleEngineBase::save_rule_engine(rule_obj);
    return true;
}


uint16_t Unit::activeModules() {
    uint16_t ret = 0;
    for (auto module: module_list) {
        if (module -> getRelayState()) ret++;
    }
    return ret;
}

/**
 * @brief Get a std::pair containing the epoch time that this method was last called and the current epoch time.
 * 
 * @return std::pair<uint64_t, uint64_t> 
 */
std::pair<uint64_t, uint64_t> Unit::getSerializationPeriod() {
    auto now = getTime();
    auto ret = std::make_pair(last_serialization, now);

    last_serialization = now;

    return ret;
}