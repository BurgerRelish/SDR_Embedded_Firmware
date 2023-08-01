#pragma once

#ifndef SDR_UNIT_H
#define SDR_UNIT_H

#include <stdint.h>

#include "RuleStore.h"
#include "TagSearch.h"

#include "../data_containers/ps_string.h"

class SDRUnit: public TagSearch, public RuleStore {
    private:
    double total_active_power;
    double total_reactive_power;
    double total_apparent_power;
    bool power_status;
    int number_of_modules;

    ps_string unit_id_;

    public:
    SDRUnit(const std::string unit_id, const int module_count, const std::vector<std::string>& tag_list, const ps_string& nvs_tag) : 
    number_of_modules(module_count),
    total_active_power(0),
    total_reactive_power(0),
    total_apparent_power(0),
    power_status(false),
    TagSearch(tag_list, nvs_tag),
    RuleStore(nvs_tag)
    {
        unit_id_ <<= unit_id;
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

    const int& moduleCount() {
        return number_of_modules;
    }

    const ps_string& id() {
        return unit_id_;
    }

};

#endif