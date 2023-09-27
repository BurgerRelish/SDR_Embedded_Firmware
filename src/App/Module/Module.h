#pragma once

#ifndef SDR_MODULE_H
#define SDR_MODULE_H

#define READING_DEQUE_SIZE 15

#include <ArduinoJson.h>
#include <ps_stl.h>

#include "Persistence.h"

#include "SDRSemantics.h"
#include "RuleEngineBase.h"
#include "ModuleInterface.h"

#include "Reading.h"
#include "StatusChange.h"

struct ReadingPacket {
    uint8_t status;
    float voltage;
    float frequency;
    float current;
    float active_power;
    float power_factor;
    float energy_usage;
};



class Module : public re::RuleEngineBase, public std::enable_shared_from_this<Module> {
    private:
    std::shared_ptr<ModuleInterface> interface;
    std::shared_ptr<re::FunctionStorage> functions;
    uint16_t slave_address;
    bool update_required;
    bool save_required;

    ps::deque<Reading> readings;
    

    ps::deque<StatusChange> status_updates;

    ps::string module_id;
    int circuit_priority;


    template <typename T>
    const T calc_max(const T Reading::*, const ps::deque<Reading>&) const;
    template <typename T>
    const T calc_min(const T Reading::*, const ps::deque<Reading>&) const;
    template <typename T>
    const T calc_mode(const T Reading::*, const ps::deque<Reading>&) const;
    template <typename T>
    const T calc_mean(const T Reading::*, const ps::deque<Reading>&) const;
    template <typename T>
    const T calc_stddev(const T Reading::*, const ps::deque<Reading>&) const;
    template <typename T>
    const T calc_iqr(const T Reading::*, const ps::deque<Reading>&) const;
    template <typename T>
    const T calc_kurt(const T, const T Reading::*, const ps::deque<Reading>&)  const;

    std::tuple<double, double, double, double> get_summary(double Reading::*, ps::deque<Reading>&);

    void load_re_vars();
    uint64_t getTime();

    public:
    uint16_t new_readings = 0;
    uint16_t new_status_changes = 0;
    
    Module(std::shared_ptr<re::FunctionStorage> functions, std::shared_ptr<ModuleInterface> interface, uint16_t address, ps::string id, uint16_t firmware_version, uint16_t hardware_version) : 
    RuleEngineBase(MODULE_TAG_LIST, functions),
    module_id(id),
    interface(interface),
    slave_address(address),
    update_required(true),
    save_required(false),
    new_readings(0)
    {
        load_re_vars();
    }

    Module(std::shared_ptr<re::FunctionStorage> functions, const ps::string& id, const int& priority, const uint8_t& address, std::shared_ptr<ModuleInterface> _interface, bool update_required = false) :
    module_id(id),
    slave_address(address),
    interface(_interface),
    circuit_priority(priority),
    update_required(update_required),
    save_required(false),
    new_readings(0),
    RuleEngineBase(MODULE_TAG_LIST, functions)
    {
        load_re_vars();
    }
    
    bool load(JsonObject&);
    bool save(JsonObject&);
    bool serialize(JsonObject&);

    bool refresh();
    bool setRelayState(bool);
    const bool getRelayState();
    const uint64_t getRelayStateChangeTime();
    const ps::deque<StatusChange>& getRelayStateChanges();

    const ps::string& getModuleID();
    const int& getModulePriority();
    const Reading& getLatestReading();
    const ps::deque<Reading>& getReadings();

    bool& updateRequired();
    bool& saveRequired();

    template <typename T>
    const T max(const T Reading::* attribute);
    template <typename T>
    const T min(const T Reading::* attribute);
    template <typename T>
    const T mode(const T Reading::* attribute);
    template <typename T>
    const T mean(const T Reading::* attribute);
    template <typename T>
    const T stddev(const T Reading::* attribute);
    template <typename T>
    const T iqr(const T Reading::* attribute);
    template <typename T>
    const T kurt(const T Reading::* attribute);

};

#include "Module.inl"

#endif