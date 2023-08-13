#pragma once

#ifndef SDR_MODULE_H
#define SDR_MODULE_H

#define READING_DEQUE_SIZE 15

#include <ArduinoJson.h>
#include <ps_stl.h>

#include <ps_stl.h>

#include "../rule_engine/RuleEngineBase.h"
#include "Reading.h"
#include "../hardware_interface/Persistence.h"
#include "sdr_semantics.h"

namespace sdr {

struct StatusChange {
    bool state;
    uint64_t timestamp;
};


class Module : public re::RuleEngineBase, public std::enable_shared_from_this<Module> {
    private:
    ps::deque<Reading> readings;
    ps::deque<Reading> new_readings;

    ps::vector<StatusChange> _status;

    ps::string module_id;
    int circuit_priority;
    bool state_changed;
    bool relay_status;
    uint64_t switch_time;

    uint8_t swi_address = 0;
    uint8_t io_offset = 0;

    bool update;
    bool save;

    template <typename T>
    T calc_max(T Reading::* attribute, ps::deque<Reading>& _readings);
    template <typename T>
    T calc_min(T Reading::* attribute, ps::deque<Reading>& _readings);
    template <typename T>
    T calc_mode(T Reading::* attribute, ps::deque<Reading>& _readings);
    template <typename T>
    T calc_mean(T Reading::* attribute, ps::deque<Reading>& _readings);
    template <typename T>
    T calc_stddev(T Reading::* attribute, ps::deque<Reading>& _readings);
    template <typename T>
    T calc_iqr(T Reading::* attribute, ps::deque<Reading>& _readings);
    template <typename T>
    T calc_kurt(T mean, T Reading::* attribute, ps::deque<Reading>& _readings);

    std::tuple<double, double, double, double> getSummarization(double Reading::* attribute);

    template <std::size_t I = 0, typename... Ts>
    typename std::enable_if<I == sizeof...(Ts),
                    void>::type
    loadArray(std::tuple<Ts...> tup, JsonArray& array)
    {
        return;
    }
    
    template <std::size_t I = 0, typename... Ts>
    typename std::enable_if<(I < sizeof...(Ts)),
                    void>::type
    loadArray(std::tuple<Ts...> tup, JsonArray& array)
    {
        array.add(std::get<I>(tup));        
        loadArray<I + 1>(tup, array);
    }

    void load_re_vars();

    public:
    Module(std::shared_ptr<re::FunctionStorage> functions, const ps::string& id, const int& priority, const std::vector<std::string>& tag_list, const uint8_t& address, const uint8_t& offset, bool update_required = false) :
    module_id(id),
    swi_address(address),
    io_offset(offset),
    circuit_priority(priority),
    switch_time(0),
    relay_status(false),
    update(update_required),
    save(false),
    state_changed(false),
    RuleEngineBase(MODULE_TAG_LIST, functions)
    {
        load_re_vars();
    }

    void loadUpdate(JsonObject& update_obj);
    bool serializeStatusChange(JsonArray& array);
    bool serializeReadings(JsonArray& reading_array);
    void saveParameters(Persistence<fs::LittleFSFS>& nvs);

    const ps::string& id() {
        return module_id;
    }

    const int& priority() {
        return circuit_priority;
    }

    const uint8_t& address() {
        return swi_address;
    }

    const uint8_t& offset() {
        return io_offset;
    }

    const bool& status() {
        return relay_status;
    }

    const uint64_t& switchTime() {
        return switch_time;
    }

    bool statusChanged() {
        return state_changed;
    }

    /**
     * @brief Get the Status Changes object. Will be empty after serializeStatusChanges() has been called,
     * 
     * @return ps::vector<StatusChange>& 
     */
    ps::vector<StatusChange>& getStatusChanges() {
        return _status;
    }

    void newStatusChange(StatusChange new_status) {
        _status.push_back(new_status);
        relay_status = new_status.state;
        switch_time = new_status.timestamp;
        state_changed = false;
    }

    void setRelayState(bool state) {
        if (state == relay_status) return;
        state_changed = true;
        relay_status = state;
    }

    /**
     * @brief Adds a new reading to the module vector. Pops a reading off the back if the deque is too big.
    */
    void add_reading(const Reading& reading) {
        readings.emplace_front(reading);
        new_readings.emplace_front(reading);

        if (readings.size() > READING_DEQUE_SIZE) {
            readings.pop_back();
        }
    }

    /**
     * @brief Get the latest reading object.
     * 
     * @return const Reading& 
     */
    const Reading& latestReading() {
        return readings.front();
    }

    /**
     * @brief Get the Readings object deque. May be empty after serialization, rather use `latestReading()` to get the latest reading.
     * 
     * @return ps::deque<Reading>& Reference to reading deque.
     */
    const ps::deque<Reading>& getReadings() {
        return readings;
    }

    /**
     * @brief Check whether the module requires an update.
     * 
     * @return true - Update required.
     * @return false - Update not required.
     */
    bool& updateRequired() {
        return update;
    }
    
    /**
     * @brief Check whether the module parameters require saving.
     * 
     * @return true 
     * @return false 
     */
    bool& saveRequired() {
        return save;
    }

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