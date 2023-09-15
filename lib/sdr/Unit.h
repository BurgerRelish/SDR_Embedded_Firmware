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
    std::shared_ptr<re::FunctionStorage> functions;

    uint8_t power_sense_pin;

    double total_active_power;
    double total_reactive_power;
    double total_apparent_power;
    double mean_pf;
    double mean_voltage;
    double mean_frequency;
    
    bool power_status;
    
    uint16_t active_modules;
    uint16_t number_of_modules;

    ps::string unit_id_;

    bool update;
    bool save;

    void load_vars();
    void loadUnitVarsInModule(std::shared_ptr<Module>& module);

    public:
    Unit(std::shared_ptr<re::FunctionStorage> functions, const std::string unit_id, uint8_t power_sense) : 
    total_active_power(0),
    total_reactive_power(0),
    total_apparent_power(0),
    power_status(false),
    save(false),
    power_sense_pin(power_sense),
    functions(functions),
    RuleEngineBase(UNIT_TAG_LIST, functions)
    {
        unit_id_ <<= unit_id;
    }

    void begin(Stream* stream_1, uint8_t ctrl_1, uint8_t dir_1, Stream* stream_2, uint8_t ctrl_2, uint8_t dir_2);

    bool& updateRequired() {return update; }
    bool& saveRequired() { return save; }
    void saveParameters(Persistence<fs::LittleFSFS>& nvs);
    void loadUpdate(JsonObject& update_obj);

    bool evaluateAll();
    bool evaluateModules();

    const ps::string& id() { return unit_id_; }
    uint16_t& moduleCount() { return number_of_modules; }

    double& totalActivePower() { return total_active_power; }
    double& totalReactivePower() { return total_reactive_power; }
    double& totalApparentPower() { return total_apparent_power; }
    double& meanVoltage() { return mean_voltage; }
    double& meanFrequency() { return mean_frequency; }
    double& meanPowerFactor() { return mean_pf; }
    bool& powerStatus() { return power_status; }    
    uint16_t activeModules();

    bool refresh();

    void load_unit() {

    }

};


#endif