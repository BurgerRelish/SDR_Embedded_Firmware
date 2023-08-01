#pragma once

#ifndef SDR_MODULE_H
#define SDR_MODULE_H

#include <string>
#include <stdint.h>
#include <vector>

#include "../data_containers/ps_stack.h"

#include "RuleStore.h"
#include "TagSearch.h"

struct Reading {
    double voltage;
    double frequency;
    double active_power;
    double reactive_power;
    double apparent_power;
    double power_factor;
    double kwh_usage;
    uint64_t timestamp;
};

struct StatusChange {
    bool state;
    uint64_t timestamp;
};

class Module : public TagSearch, public RuleStore {
    private:
    ps_stack<Reading> readings;
    ps_stack<StatusChange> _status;

    ps_string module_id;
    int circuit_priority;

    bool relay_status;
    uint64_t switch_time;

    uint8_t i2c_address = 0;
    uint8_t io_offset = 0;

    public:
    Module(const std::string& id, const std::vector<std::string>& tag_list, const int& priority, const uint8_t& address, const uint8_t& offset, const ps_string& nvs_tag) :
    TagSearch(tag_list, nvs_tag),
    RuleStore(nvs_tag),
    i2c_address(address),
    io_offset(offset),
    circuit_priority(priority),
    switch_time(0),
    relay_status(false)
    {
        module_id <<= id; // Copy ID to PSRAM.
    }

    const ps_string& id() {
        return module_id;
    }

    const int& priority() {
        return circuit_priority;
    }

    const uint8_t& address() {
        return i2c_address;
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
        return (_status.size() > 0);
    }

    ps_stack<StatusChange>& getStatusChanges() {
        return _status;
    }

    void newStatusChange(StatusChange new_status) {
        _status.push(new_status);
        relay_status = new_status.state;
        switch_time = new_status.timestamp;
    }   

    /**
     * @brief Adds a new reading to the module stack.
    */
    void addReading(const Reading& reading) {
        readings.push(reading);
    }

    /**
     * @brief Returns the last reading added to the stack.
    */
    const Reading& latestReading() {
        return readings.top();
    }

    ps_stack<Reading>& getReadings() {
        return readings;
    }
};

#endif