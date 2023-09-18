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
    uint16_t new_readings;

    ps::deque<StatusChange> status_updates;

    ps::string module_id;
    int circuit_priority;


    template <typename T>
    T calc_max(T Reading::*, ps::deque<Reading>&);
    template <typename T>
    T calc_min(T Reading::*, ps::deque<Reading>&);
    template <typename T>
    T calc_mode(T Reading::*, ps::deque<Reading>&);
    template <typename T>
    T calc_mean(T Reading::*, ps::deque<Reading>&);
    template <typename T>
    T calc_stddev(T Reading::*, ps::deque<Reading>&);
    template <typename T>
    T calc_iqr(T Reading::*, ps::deque<Reading>&);
    template <typename T>
    T calc_kurt(T, T Reading::*, ps::deque<Reading>&);

    std::tuple<double, double, double, double> get_summary(double Reading::*);

    void load_re_vars();
    uint64_t getTime();

    public:
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
    
    void load(JsonObject&);
    void save(JsonObject&);
    void loadFromArray(JsonArray&);
    void saveToArray(JsonArray&);
    bool serialize(JsonArray&);

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
    T max(T Reading::* attribute);
    template <typename T>
    T min(T Reading::* attribute);
    template <typename T>
    T mode(T Reading::* attribute);
    template <typename T>
    T mean(T Reading::* attribute);
    template <typename T>
    T stddev(T Reading::* attribute);
    template <typename T>
    T iqr(T Reading::* attribute);
    template <typename T>
    T kurt(T Reading::* attribute);

};

#include "Module.inl"

#endif