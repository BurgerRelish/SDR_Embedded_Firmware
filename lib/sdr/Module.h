#pragma once

#ifndef SDR_MODULE_H
#define SDR_MODULE_H

#define READING_DEQUE_SIZE 15

#include <ArduinoJson.h>
#include <ps_stl.h>

#include "../hardware_interface/Persistence.h"
#include "sdr_semantics.h"
#include "../rule_engine/RuleEngineBase.h"

#include "Reading.h"
#include "StatusChange.h"


namespace sdr {


class Module : public re::RuleEngineBase, public std::enable_shared_from_this<Module> {
    private:
    ps::deque<Reading> readings;
    ps::deque<Reading> new_readings;

    ps::vector<StatusChange> status_updates;

    ps::string module_id;
    int circuit_priority;

    uint8_t slave_address = 0;
    uint8_t interface = 0;

    bool update;
    bool save;

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

    public:
    Module(std::shared_ptr<re::FunctionStorage> functions, const ps::string& id, const int& priority, const uint8_t& address, const uint8_t& _interface, bool update_required = false) :
    module_id(id),
    slave_address(address),
    interface(_interface),
    circuit_priority(priority),
    update(update_required),
    save(false),
    RuleEngineBase(MODULE_TAG_LIST, functions)
    {
        load_re_vars();
    }

    void load(JsonObject&);
    void save(JsonObject&);
    bool serialize(JsonArray&);

    bool refresh();
    
    const ps::string& id();
    const int& priority();
    const uint8_t& address();
    const uint8_t& offset();
    const bool& status();
    const uint64_t& switch_time();
    bool status_changed();
    ps::vector<StatusChange>& get_status_changes();
    void new_status_change(StatusChange);
    void set_relay_state(bool);
    void add_reading(const Reading&);
    const Reading& latestReading();
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

}

#include "Module.inl"

#endif