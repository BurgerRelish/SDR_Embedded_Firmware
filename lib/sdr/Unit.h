#pragma once

#ifndef SDR_UNIT_H
#define SDR_UNIT_H

#include <stdint.h>
#include <ArduinoJson.h>

#include <ps_stl.h>

#include "../rule_engine/RuleEngineBase.h"
#include "../hardware_interface/Persistence.h"
#include "sdr_semantics.h"
#include "json_fields.h"
#include "Module.h"
#include "../ModuleInterface/ModuleInterface.h"

class Unit: public re::RuleEngineBase, private std::enable_shared_from_this<Unit> {
    private:
    std::shared_ptr<ModuleInterface> interface_1;
    std::shared_ptr<ModuleInterface> interface_2;
    ps::vector<std::shared_ptr<Module>> module_map;

    double total_active_power;
    double total_reactive_power;
    double total_apparent_power;
    bool power_status;
    uint16_t number_of_modules;

    ps::string unit_id_;
    bool update;
    bool save;

    void load_vars();

    public:
    Unit(std::shared_ptr<re::FunctionStorage> functions, const std::string unit_id) : 
    total_active_power(0),
    total_reactive_power(0),
    total_apparent_power(0),
    power_status(false),
    save(false),
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

        if (update_obj[JSON_RULE_ACTION].as<ps::string>() == JSON_REPLACE) {
            RuleEngineBase::clear_rules();
        }
        
        if (update_obj[JSON_TAG_ACTION].as<ps::string>() == JSON_REPLACE) {
            RuleEngineBase::clear_tags();
        }

        RuleEngineBase::load_rule_engine(update_obj);

        return;
    }

    bool evaluateAll();
    bool evaluateModules();

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

    uint16_t& moduleCount() { return number_of_modules; }

    uint16_t activeModules() {
        uint16_t ret = 0;
        for (auto module: module_map) {
            if (module -> getRelayState()) ret++;
        }
        return ret;
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

    }

};


#endif