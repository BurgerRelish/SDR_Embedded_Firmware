#include "Unit.h"

void Unit::load_vars() {
    re::RuleEngineBase::set_var(re::VAR_CLASS, UNIT_CLASS, std::enable_shared_from_this<Unit>::shared_from_this());

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

void Unit::begin(Stream* stream_1, uint8_t ctrl_1, uint8_t dir_1, Stream* stream_2, uint8_t ctrl_2, uint8_t dir_2) {
    number_of_modules = 0;
    interface_1 = ps::make_shared<ModuleInterface>(stream_1, ctrl_1, dir_1);
    auto found_modules_1 = interface_1 -> begin();

    interface_2 = ps::make_shared<ModuleInterface>(stream_2, ctrl_2, dir_2);
    auto found_modules_2 = interface_2 -> begin();

    for (auto found_module : found_modules_1) {
        ESP_LOGI("Dynamic Address", "Found on Port 1: %s", &found_module.first.id[0]);
        module_map.push_back(
            ps::make_shared<Module>(functions, interface_1, found_module.second, ps::string(found_module.first.id), found_module.first.firmware_version, found_module.first.hardware_version)
        );
        number_of_modules++;
    }

    for (auto found_module : found_modules_2) {
        ESP_LOGI("Dynamic Address", "Found on Port 2: %s", &found_module.first.id[0]);
        module_map.push_back(
            ps::make_shared<Module>(functions, interface_2, found_module.second, ps::string(found_module.first.id), found_module.first.firmware_version, found_module.first.hardware_version)
        );
        number_of_modules++;
    }

    
}

bool Unit::evaluateAll() {
    try {
        RuleEngineBase::reason();
    } catch (...) {
        ESP_LOGE("RULE_ENGINE", "Something went wrong evaluating unit.");
        return false;
    }

    return evaluateModules();
}

bool Unit::evaluateModules() {
    try {
        for (auto module : module_map) {
            module -> reason();
        }
    } catch (...) {
        ESP_LOGE("RULE_ENGINE", "Something went wrong evaluating modules.");
        return false;
    }

    return true;
}

bool Unit::refresh() {
    total_apparent_power = 0;
    total_active_power = 0;
    total_reactive_power = 0;
    mean_pf = 0;
    mean_voltage = 0;
    mean_frequency = 0;

    active_modules = activeModules();

    for (auto module : module_map) {
        module -> refresh();
        auto reading = module -> getLatestReading();
        total_apparent_power += reading.apparent_power;
        total_reactive_power += reading.reactive_power();
        total_active_power += reading.active_power();

        mean_pf += reading.power_factor / active_modules;
        mean_voltage += reading.voltage / number_of_modules;
        mean_frequency += reading.frequency / number_of_modules;
    }

    power_status = (analogRead(power_sense_pin) > 1000);
    return true;
}

void Unit::saveParameters(Persistence<fs::LittleFSFS>& nvs) {
    auto unit_obj = nvs.document.createNestedObject();
    unit_obj[JSON_UNIT_UID] = unit_id_.c_str();

    save_required = false;
    RuleEngineBase::save_rule_engine(unit_obj);
}

void Unit::loadParameters(Persistence<fs::LittleFSFS>& nvs) {
    auto array = nvs.document.as<JsonArray>();
    for (auto item : array) {
        if (item.containsKey(JSON_UNIT_UID)) {
            auto val = item.as<JsonObject>();
            RuleEngineBase::load_rule_engine(val);
            break;
        }
    }
}

void Unit::loadUpdate(JsonObject& update_obj) {
    save_required = true;
    update_required = false;

    if (update_obj[JSON_RULE_ACTION].as<ps::string>() == JSON_REPLACE) {
        RuleEngineBase::clear_rules();
    }
    
    if (update_obj[JSON_TAG_ACTION].as<ps::string>() == JSON_REPLACE) {
        RuleEngineBase::clear_tags();
    }

    RuleEngineBase::load_rule_engine(update_obj);

    return;
}

uint16_t Unit::activeModules() {
    uint16_t ret = 0;
    for (auto module: module_map) {
        if (module -> getRelayState()) ret++;
    }
    return ret;
}
