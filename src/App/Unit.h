#pragma once

#ifndef SDR_UNIT_H
#define SDR_UNIT_H

#include <stdint.h>
#include <ArduinoJson.h>
#include <time.h>
#include <ps_stl.h>

#include "RuleEngineBase.h"
#include "Persistence.h"
#include "Config.h"
#include "SDRSemantics.h"
#include "JSONFields.h"
#include "Module.h"
#include "ModuleInterface.h"


class Unit: public re::RuleEngineBase, private std::enable_shared_from_this<Unit> {
    private:
    std::shared_ptr<ModuleInterface> interface_1;
    std::shared_ptr<ModuleInterface> interface_2;
    ps::vector<std::shared_ptr<Module>> module_list;
    std::shared_ptr<re::FunctionStorage> functions;

    uint8_t power_sense_pin;

    double total_active_power = 0;
    double total_reactive_power = 0;
    double total_apparent_power = 0;
    double mean_current = 0;
    double mean_pf = 0;
    double mean_voltage = 0;
    double mean_frequency = 0;
    
    bool power_status = false;
    
    uint16_t active_modules = 0;
    uint16_t number_of_modules = 0;

    ps::string unit_id_;
    uint64_t last_serialization = 0;

    bool update_required = false;
    bool save_required = false;

    void load_vars();
    void loadUnitVarsInModule(std::shared_ptr<Module>& module);


    /* Time of Use */
    double kwh_price = -1;
    double tou_calc_hr = 0;
    bool loadTOUSchedule(DynamicPSRAMJsonDocument& doc, struct tm& timeinfo);

    public:
    bool publish_readings = false;
    uint32_t sample_period = DEFAULT_SAMPLE_PERIOD;
    uint32_t serialization_period = DEFAULT_SERIALIZATION_PERIOD;
    uint32_t mode = DEFAULT_MODE;

    ps::unordered_map<ps::string, std::shared_ptr<Module>> module_map;

    Unit(std::shared_ptr<re::FunctionStorage> functions, const std::string unit_id, uint8_t power_sense) : 
    total_active_power(0),
    total_reactive_power(0),
    total_apparent_power(0),
    power_status(false),
    save_required(false),
    update_required(false),
    power_sense_pin(power_sense),
    functions(functions),
    RuleEngineBase(UNIT_TAG_LIST, functions)
    {
        unit_id_ <<= unit_id;
    }

    void begin(Stream* stream_1, uint8_t ctrl_1, uint8_t ctrl_2, uint8_t dir_1);

    bool evaluateAll();
    bool evaluateModules();

    void create_module_map();

    double getkWhPrice();

    const ps::string& id() { return unit_id_; }
    uint16_t& moduleCount() { return number_of_modules; }
    double& totalActivePower() { return total_active_power; }
    double& totalReactivePower() { return total_reactive_power; }
    double& totalApparentPower() { return total_apparent_power; }
    double& meanVoltage() { return mean_voltage; }
    double& meanFrequency() { return mean_frequency; }
    double& meanPowerFactor() { return mean_pf; }
    double& meanCurrent() { return mean_current; }
    bool powerStatus() { return (analogRead(power_sense_pin) > 150); }    
    uint16_t activeModules();
    ps::vector<std::shared_ptr<Module>>& getModules() { return module_list; }
    bool refresh();
    uint64_t getTimeSinceLastSerialization() { return getTime() - last_serialization; }
    std::pair<uint64_t, uint64_t> getSerializationPeriod();

    uint64_t getTime() {
        struct tm timeinfo;
        if (!getLocalTime(&timeinfo)) {
            //Serial.println("Failed to obtain time");
            return 0;
        }

        return (uint64_t) mktime(&timeinfo);
    }   

    void publishReadings() {publish_readings = true;}
    bool load(JsonObject& obj);
    bool save(JsonObject& obj);
};


#endif