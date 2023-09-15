#include "Unit.h"

void Unit::load_vars() {
    re::RuleEngineBase::set_var(re::VAR_CLASS, UNIT_CLASS, std::enable_shared_from_this<Unit>::shared_from_this());

    re::RuleEngineBase::set_var(re::VAR_DOUBLE, TOTAL_ACTIVE_POWER, std::function<double()>([this]() { return this->totalActivePower(); }));
    re::RuleEngineBase::set_var(re::VAR_DOUBLE, TOTAL_REACTIVE_POWER, std::function<double()>([this]() { return this->totalReactivePower(); }));
    re::RuleEngineBase::set_var(re::VAR_DOUBLE, TOTAL_APPARENT_POWER, std::function<double()>([this]() { return this->totalApparentPower(); }));
    re::RuleEngineBase::set_var(re::VAR_BOOL, POWER_STATUS, std::function<bool()>([this]() { return this->powerStatus(); }));
    re::RuleEngineBase::set_var(re::VAR_STRING, UNIT_ID, std::function<ps::string()>([this]() { return this->id(); }));
    re::RuleEngineBase::set_var(re::VAR_ARRAY, UNIT_TAG_LIST, std::function<ps::vector<ps::string>()>([this]() { return this->get_tags(); }));
    re::RuleEngineBase::set_var(re::VAR_INT, MODULE_COUNT, std::function<int()>([this]() { return this->moduleCount(); }));
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