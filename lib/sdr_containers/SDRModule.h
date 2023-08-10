#pragma once

#ifndef SDR_MODULE_H
#define SDR_MODULE_H

#include <string>
#include <stdint.h>
#include <vector>
#include <tuple>
#include <LittleFS.h>

#include "../ps_stl/ps_stl.h"

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

        Reading(ps::queue<double>& var, uint64_t ts) {
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
    ps::vector<Reading> readings;
    Reading latest_reading;
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

    /**
     * @brief Gets the maximum value of the attribute
     * 
     * @param attribute 
     * @return double 
     */
    double calculateMaximum(double Reading::* attribute) {
        double max = 0;
        for (auto& reading : readings) {
            if(reading.*attribute > max) max = reading.*attribute;
        }
        return max;
    }

    /**
     * @brief Gets the mean value of the attribute.
     * 
     * @param attribute 
     * @return double 
     */
    double calculateMean(double Reading::* attribute) {
        double ret;

        for (auto reading : readings) {
            ret += reading.*attribute;
        }

        return (ret / readings.size());
    }

    /**
     * @brief Calculates the IQR of the provided attribute.
     * 
     * @param data 
     * @param attribute 
     * @return double 
     */
    double calculateIQR(double Reading::* attribute) {
        ps::vector<double> attributeValues;
        for (auto& reading : readings) {
            attributeValues.push_back(reading.*attribute);
        }

        size_t n = attributeValues.size();
        std::sort(attributeValues.begin(), attributeValues.end());

        size_t q1_index = n / 4;
        size_t q3_index = (3 * n) / 4;

        double q1_value = (attributeValues[q1_index - 1] + attributeValues[q1_index]) / 2.0;
        double q3_value = (attributeValues[q3_index - 1] + attributeValues[q3_index]) / 2.0;

        return q3_value - q1_value;
    }

    /**
     * @brief Calculates the standard deviation of the provided attribute.
     * 
     * @param mean 
     * @param attribute 
     * @return double 
     */
    double calculateStandardDeviation(double mean, double Reading::* attribute) {
        double variance = 0;

        for (auto& reading : readings) {
            variance += pow((reading.*attribute - mean), 2);
        }

        variance /= (readings.size() - 1);

        return sqrt(variance);

    }

    /**
     * @brief Calculates the Kurtosis of the provided attribute.
     * 
     * @param mean 
     * @param attribute 
     * @return double 
     */
    double calculateKurtosis(double mean, double Reading::* attribute) {
        float variance = pow(calculateStandardDeviation(mean, attribute), 2);

        auto n = readings.size();

        float C1 = ((n + 1) * n) / ((n - 1) * (n - 2) * (n - 3));
        float C2 = (-3 * pow(n - 1, 2)) / ((n - 2) * (n - 3));

        float C3 = 0;
        for (auto& reading : readings) {
            C3 += pow(reading.*attribute - mean, 4);
        }

        return (C1 * (C3 / variance) - C2);
    }

    /**
     * @brief Get the Statistical Summarization of the requested attribute.
     * 
     * @param attribute 
     * @return std::tuple<double, double, double, double> Mean, Maximum, IQR, Kurtosis
     */
    std::tuple<double, double, double, double> getSummarization(double Reading::* attribute) {
        auto mean = calculateMean(attribute);
        return std::make_tuple(
            mean,
            calculateMaximum(attribute),
            calculateIQR(attribute),
            calculateKurtosis(mean, attribute)
        );
    }

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
     * @brief Serializes the readings into the provided array. Automatically appends a new object to the array.
     * 
     * @return bool true - If serialization was successful, or the class is empty, else false.
     */
    bool serializeReadings(JsonArray& reading_array) {
        if (!readings.size()) return true;

        auto document = reading_array.createNestedObject();

        document["moduleID"] = module_id.c_str();
        document["count"] = readings.size();

        /* Load calculated values into json arrays.*/
        {        
            auto voltage_array = document.createNestedArray("voltage");
            loadArray(getSummarization(&Reading::voltage), voltage_array);
        }

        {
            auto frequency_array = document.createNestedArray("frequency");
            loadArray(getSummarization(&Reading::frequency), frequency_array);
        }

        {
            auto apparent_power_array = document.createNestedArray("apparentPower");
            loadArray(getSummarization(&Reading::apparent_power), apparent_power_array);
        }

        {
            auto power_factor_array = document.createNestedArray("powerFactor");
            loadArray(getSummarization(&Reading::power_factor), power_factor_array);
        }
    

        {        
            double kwh_sum = 0;
            ps::vector<uint64_t> ts_vector;
            for (auto& reading : readings) {
                kwh_sum += reading.kwh_usage;
                ts_vector.push_back(reading.timestamp);
            }

            document["kwh"] = kwh_sum;

            std::sort(ts_vector.begin(), ts_vector.end());
            
            auto timestamp_arr = document.createNestedArray("timestamp");
            timestamp_arr.add(ts_vector.front());
            timestamp_arr.add(ts_vector.back());
        }

        readings.clear();
        return true;
    }

    /**
     * @brief Serializes all the status changes on the module. Clears the status change stack.
     * 
     * @return bool true - If serialization was successful, or the class is empty, else false.
     */
    bool serializeStatusChange(JsonArray& array) {
        if (!_status.size()) return true;

        auto document = array.createNestedObject();
        
        document["moduleID"] = module_id.c_str();
        auto state_arr = document.createNestedArray("state");
        auto ts_arr = document.createNestedArray("timestamp");

        while(!_status.empty()) {
            state_arr.add(_status.back().state);
            ts_arr.add(_status.back().timestamp);
            _status.pop_back();
        }

        return true;
    }

    /**
     * @brief Loads the provided update object into the class.
     * 
     * @note JSON Format:
     * {
     *  "action" : "replace" or "append",
     *  "priority" : int,
     *  "rules" : [{
     *      "priority" : int,
     *      "expression" : "string",
     *      "command" : "string"
     *      }, ...],
     *  "tags" : ["string", ...]
     * }
     * 
     * @param update_obj 
     */
    void loadUpdate(JsonObject update_obj) {
        save = true;
        update = false;

        auto rule_arr = update_obj["rules"].as<JsonArray>();
        ps::vector<Rule> rule_vect;
        for (auto rule : rule_arr) {
            rule_vect.push_back(
                Rule{
                    rule["priority"].as<int>(),
                    rule["expression"].as<ps::string>(),
                    rule["command"].as<ps::string>()
                }
            );
        }

        auto tag_arr = update_obj["tags"].as<JsonArray>();
        ps::vector<ps::string> tag_vect;
        for (auto tag : tag_arr) {
            tag_vect.push_back(
                tag.as<ps::string>()
            );
        }

        if (update_obj["action"].as<ps::string>() == "replace") {
            replaceRules(rule_vect);
            replaceTag(tag_vect);
        } else {
            appendRule(rule_vect);
            appendTag(tag_vect);
        }

        circuit_priority = update_obj["priority"].as<int>();

        return;
    }

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
     * @brief Adds a new reading to the module vector.
    */
    void addReading(const Reading& reading) {
        latest_reading = reading;
        readings.push_back(reading);
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
     * @brief Get the Readings object vector. May be empty after serialization, rather use `latestReading()` to get the latest reading.
     * 
     * @return ps::vector<Reading>& Reference to reading vector.
     */
    ps::vector<Reading>& getReadings() {
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