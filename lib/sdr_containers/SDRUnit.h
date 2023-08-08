#pragma once

#ifndef SDR_UNIT_H
#define SDR_UNIT_H

#include <stdint.h>
#include <ArduinoJson.h>

#include "RuleStore.h"
#include "TagSearch.h"

#include "../data_containers/ps_string.h"
#include "../data_containers/ps_vector.h"

#include "../SDR/Persistence.h"

class SDRUnit: public TagSearch, public RuleStore {
    private:
    double total_active_power;
    double total_reactive_power;
    double total_apparent_power;
    bool power_status;
    int number_of_modules;

    ps_string unit_id_;
    bool update;
    bool save;

    public:
    SDRUnit(const std::string unit_id, const int module_count, const std::vector<std::string>& tag_list, const std::vector<Rule> rule_list, bool update_required = false) : 
    number_of_modules(module_count),
    total_active_power(0),
    total_reactive_power(0),
    total_apparent_power(0),
    power_status(false),
    save(false),
    update(update_required),
    TagSearch(tag_list),
    RuleStore(rule_list)
    {
        unit_id_ <<= unit_id;
    }

    void saveParameters(Persistence<fs::LittleFSFS>& nvs) {
        auto unit_obj = nvs.document.createNestedObject();
        unit_obj["uid"] = unit_id_.c_str();
        auto tag_arr = unit_obj["tags"].createNestedArray();
        saveTags(tag_arr);

        auto rule_arr = unit_obj["rules"].createNestedArray();
        saveRules(rule_arr);

        save = false;
    }

    void loadUpdate(JsonObject& update_obj) {
        save = true;
        update = false;

        auto rule_arr = update_obj["rules"].as<JsonArray>();
        ps_vector<Rule> rule_vect;
        for (auto rule : rule_arr) {
            rule_vect.push_back(
                Rule{
                    rule["priority"].as<int>(),
                    rule["expression"].as<ps_string>(),
                    rule["command"].as<ps_string>()
                }
            );
        }

        auto tag_arr = update_obj["tags"].as<JsonArray>();
        ps_vector<ps_string> tag_vect;
        for (auto tag : tag_arr) {
            tag_vect.push_back(
                tag.as<ps_string>()
            );
        }

        if (update_obj["action"].as<ps_string>() == "replace") {
            replaceRules(rule_vect);
            replaceTag(tag_vect);
        } else {
            appendRule(rule_vect);
            appendTag(tag_vect);
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

    const ps_string& id() {
        return unit_id_;
    }

    bool& updateRequired() {
        return update;
    }
    
    bool& saveRequired() {
        return save;
    }

};

#endif