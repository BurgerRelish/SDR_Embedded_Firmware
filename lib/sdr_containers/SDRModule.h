#pragma once

#ifndef SDR_MODULE_H
#define SDR_MODULE_H

#include <string>
#include <stdint.h>
#include <vector>
#include <LittleFS.h>

#include "../data_containers/ps_stack.h"
#include "../data_containers/ps_queue.h"

#include "../SDR/Persistence.h"

#include "RuleStore.h"
#include "TagSearch.h"

class Reading {
    public:
        double voltage;
        double frequency;
        double active_power;
        double reactive_power;
        double apparent_power;
        double power_factor;
        double kwh_usage;
        uint64_t timestamp;

        Reading(ps_queue<double>& var, uint64_t ts) {
            voltage = var.front();
            var.pop();
            frequency = var.front();
            var.pop();
            active_power = var.front();
            var.pop();
            reactive_power = var.front();
            var.pop();
            apparent_power = var.front();
            var.pop();
            power_factor = var.front();
            var.pop();
            kwh_usage = var.front();
            var.pop();

            timestamp = ts;
        }

        Reading(double v, double fr, double ap, double rp, double sp, double pf, double kwh, uint64_t ts) :
        voltage(v),
        frequency(fr),
        active_power(ap),
        reactive_power(rp),
        apparent_power(sp),
        power_factor(pf),
        kwh_usage(kwh),
        timestamp(ts)
        {}
};

struct StatusChange {
    bool state;
    uint64_t timestamp;
};


class Module : public TagSearch, public RuleStore {
    private:
    ps_stack<Reading> readings;
    Reading latest_reading;
    ps_stack<StatusChange> _status;

    ps_string module_id;
    int circuit_priority;

    bool state_changed;
    bool relay_status;
    uint64_t switch_time;

    uint8_t swi_address = 0;
    uint8_t io_offset = 0;

    bool update;
    bool save;

    public:
    Module(const std::string& id, const int& priority, const std::vector<std::string>& tag_list, const std::vector<Rule>& rule_list, const uint8_t& address, const uint8_t& offset, bool update_required = false) :
    TagSearch(tag_list),
    RuleStore(rule_list),
    swi_address(address),
    io_offset(offset),
    circuit_priority(priority),
    switch_time(0),
    relay_status(false),
    update(update_required),
    save(false),
    state_changed(false),
    latest_reading(0, 0, 0, 0, 0, 0, 0, 0)
    {
        module_id <<= id; // Copy ID to PSRAM.
    }

    /**
     * @brief Saves the module parameters to the file opened by the Persistence class.
     * 
     * @param nvs 
     */
    void saveParameters(Persistence<fs::LittleFSFS>& nvs) {
        auto module_obj = nvs.document.createNestedObject();
        module_obj["mid"] = module_id.c_str();
        module_obj["pr"] = circuit_priority;
        auto tag_arr = module_obj["tags"].createNestedArray();
        saveTags(tag_arr);

        auto rule_arr = module_obj["rules"].createNestedArray();
        saveRules(rule_arr);
        save = false;
    }

    /**
     * @brief Serializes the readings and returns a ps_string containing the JSON formatted string. Clears the readings stack.
     * 
     * @return ps_string JSON Formatted reading packet.
     */
    ps_string serializeReadings() {
        if (!readings.size()) return "";

        DynamicPSRAMJsonDocument document(6 * JSON_ARRAY_SIZE(readings.size()) + JSON_OBJECT_SIZE(7) + 128);
        document["moduleID"] = module_id.c_str();
        auto voltage_arr = document["voltage"].createNestedArray();
        auto frequency_arr = document["frequency"].createNestedArray();
        auto SP_arr = document["apparentPower"].createNestedArray();
        auto PF_arr = document["powerFactor"].createNestedArray();
        auto kwh_arr = document["kwh"].createNestedArray();
        auto ts_arr = document["timestamp"].createNestedArray();

        while (!readings.empty()) {
            voltage_arr.add(readings.top().voltage);
            frequency_arr.add(readings.top().frequency);
            SP_arr.add(readings.top().apparent_power);
            PF_arr.add(readings.top().power_factor);
            kwh_arr.add(readings.top().kwh_usage);
            ts_arr.add(readings.top().timestamp);

            readings.pop();
        }

        ps_string result;
        if(!serializeJson(document, result)) {
            ESP_LOGE("MODULE", "Serialization Failed.");
            return "";
        }
        return result;
    }

    /**
     * @brief Serializes all the status changes on the module. Clears the status change stack.
     * 
     * @return ps_string 
     */
    ps_string serializeStatusChange() {
        if (!_status.size()) return "";

        DynamicPSRAMJsonDocument document(2 * JSON_ARRAY_SIZE(_status.size()) + JSON_OBJECT_SIZE(3) + 128);

        document["moduleID"] = module_id.c_str();
        auto state_arr = document["state"].createNestedArray();
        auto ts_arr = document["timestamp"].createNestedArray();

        while(!_status.empty()) {
            state_arr.add(_status.top().state);
            ts_arr.add(_status.top().timestamp);
        }

        ps_string result;
        if(serializeJson(document, result) == 0) {
            ESP_LOGE("MODULE", "Serialization Failed.");
            return "";
        }

        return result;
    }

    void loadUpdate(JsonObject update_obj) {
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

    const ps_string& id() {
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

    ps_stack<StatusChange>& getStatusChanges() {
        return _status;
    }

    void newStatusChange(StatusChange new_status) {
        _status.push(new_status);
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
     * @brief Adds a new reading to the module stack.
    */
    void addReading(const Reading& reading) {
        latest_reading = reading;
        readings.push(reading);
    }

    /**
     * @brief Get the latest reading object.
     * 
     * @return const Reading& 
     */
    const Reading& latestReading() {
        return latest_reading;
    }

    /**
     * @brief Get the Readings object stack. May be empty after serialization, rather use `latestReading()` to get the latest reading.
     * 
     * @return ps_stack<Reading>& Reference to reading stack.
     */
    ps_stack<Reading>& getReadings() {
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
};

#endif