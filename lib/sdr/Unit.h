#pragma once

#ifndef SDR_UNIT_H
#define SDR_UNIT_H

#include <stdint.h>
#include <ArduinoJson.h>

#include "RuleStore.h"
#include "TagSearch.h"

#include <ps_stl.h>

#include "../rule_engine/RuleEngineBase.h"
#include "../hardware_interface/Persistence.h"
#include "sdr_semantics.h"


namespace sdr {

class Unit: public re::RuleEngineBase, private std::enable_shared_from_this<Unit> {
    private:
    double total_active_power;
    double total_reactive_power;
    double total_apparent_power;
    bool power_status;
    int number_of_modules;

    ps::string unit_id_;
    bool update;
    bool save;

    public:
    Unit(std::shared_ptr<re::FunctionStorage> functions, const std::string unit_id, const int module_count, const std::vector<std::string>& tag_list, const std::vector<Rule> rule_list, bool update_required = false) : 
    number_of_modules(module_count),
    total_active_power(0),
    total_reactive_power(0),
    total_apparent_power(0),
    power_status(false),
    save(false),
    update(update_required),
    RuleEngineBase(UNIT_TAG_LIST, functions)
    {
        unit_id_ <<= unit_id;
    }

    void saveParameters(Persistence<fs::LittleFSFS>& nvs) {
        auto unit_obj = nvs.document.createNestedObject();
        unit_obj["uid"] = unit_id_.c_str();
        auto tag_arr = unit_obj["tags"].createNestedArray();


        auto rule_arr = unit_obj["rules"].createNestedArray();

        save = false;
    }

    void loadUpdate(JsonObject& update_obj) {
        save = true;
        update = false;

        auto rule_arr = update_obj["rules"].as<JsonArray>();
        ps::vector<std::tuple<int, ps::string, ps::string>> rule_vect;
        for (auto rule : rule_arr) {
            rule_vect.push_back(
                std::make_tuple(
                    rule["priority"].as<int>(),
                    rule["expression"].as<ps::string>(),
                    rule["command"].as<ps::string>()
                )
            );
        }

        auto tag_arr = update_obj["tags"].as<JsonArray>();
        ps::vector<ps::string> tag_vect;
        for (auto tag : tag_arr) {
            tag_vect.push_back(
                tag.as<ps::string>()
            );
        }

        if (update_obj["action"].as<ps::string>() == "replace") {
            replace_rules(rule_vect);
            replace_tag(tag_vect);
        } else {
            add_rule(rule_vect);
            add_tag(tag_vect);
        }

        return;
    }

    double& totalActivePower() {
        return total_active_power;
    }

    double& totalReactivePower() {
        return total_reactive_power;
    }

    double& totalApparentPower() {
        return total_apparent_power;
    }

    bool& powerStatus() {
        return power_status;
    }

    int& moduleCount() {
        return number_of_modules;
    }

    const ps::string& id() {
        return unit_id_;
    }

    bool& updateRequired() {
        return update;
    }
    
    bool& saveRequired() {
        return save;
    }

    void load_unit() {
        re::RuleEngineBase::set_var(re::VAR_CLASS, UNIT_CLASS, std::enable_shared_from_this<Unit>::shared_from_this());

        re::RuleEngineBase::set_var(re::VAR_DOUBLE, TOTAL_ACTIVE_POWER, std::function<double()>([this]() { return this->totalActivePower(); }));
        re::RuleEngineBase::set_var(re::VAR_DOUBLE, TOTAL_REACTIVE_POWER, std::function<double()>([this]() { return this->totalReactivePower(); }));
        re::RuleEngineBase::set_var(re::VAR_DOUBLE, TOTAL_APPARENT_POWER, std::function<double()>([this]() { return this->totalApparentPower(); }));
        re::RuleEngineBase::set_var(re::VAR_BOOL, POWER_STATUS, std::function<bool()>([this]() { return this->powerStatus(); }));
        re::RuleEngineBase::set_var(re::VAR_STRING, UNIT_ID, std::function<ps::string()>([this]() { return this->id(); }));
        re::RuleEngineBase::set_var(re::VAR_ARRAY, UNIT_TAG_LIST, std::function<ps::vector<ps::string>()>([this]() { return this->get_tags(); }));
        re::RuleEngineBase::set_var(re::VAR_INT, MODULE_COUNT, std::function<int()>([this]() { return this->moduleCount(); }));
    }

};

} // namespace SDR

#endif