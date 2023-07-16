#pragma once

#ifndef SDR_CONTAINERS_H
#define SDR_CONTAINERS_H

#include <exception>
#include <string>
#include <vector>
#include "ps_vector.h"
#include "ps_stack.h"
#include "ps_string.h"
#include <Preferences.h>
#include <iostream>
#include <sstream>

class SDRException : public std::exception {
public:
    SDRException(const std::string& message) : message_(message) {}

    const char* what() const noexcept override {
        return message_.c_str();
    }

private:
    std::string message_;
};

struct Rule {
    int priority;
    ps_string expression;
    ps_string command;
};

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

class RuleStore {
    private:
        ps_vector<Rule> rule_store;
        ps_string _nvs_tag;

        void loadRules() {
            Preferences nvs;

            nvs.begin(_nvs_tag.c_str());
            uint32_t rule_count = nvs.getUInt("/count", 0);
            std::ostringstream path;

            for (size_t i = 0; i < rule_count; i++) {
                Rule new_rule;

                path << "/p/" << i;
                new_rule.priority = nvs.getUInt(path.str().c_str());
                path.clear();

                path << "/e/" << i;
                new_rule.expression = nvs.getString(path.str().c_str()).c_str();
                path.clear();

                path << "/c/" << i;
                new_rule.command = nvs.getString(path.str().c_str()).c_str();
                path.clear();

                rule_store.push_back(new_rule);        
            }

            nvs.end();         
        }

        void saveRules() {
            Preferences nvs;

            nvs.begin(_nvs_tag.c_str());
            std::ostringstream path;

            nvs.putUInt("/count", rule_store.size());

            for (size_t i = 0; i < rule_store.size(); i++) {
                Rule rule = rule_store.at(i);

                path << "/p/" << i;
                nvs.putInt(path.str().c_str(), rule.priority);
                path.clear();

                path << "/e/" << i;
                nvs.putString(path.str().c_str(), rule.expression.c_str());
                path.clear();

                path << "/c/" << i;
                nvs.putString(path.str().c_str(), rule.command.c_str());
                path.clear();
            }

            nvs.end();
        }
    
    public:
        RuleStore(const ps_string& nvs_tag): _nvs_tag(nvs_tag) {
            loadRules();
        }

        const ps_vector<Rule>& getRules() {
            return rule_store;
        }

        void clearRules() {
            rule_store.clear();

            Preferences nvs;
            nvs.begin(_nvs_tag.c_str());
            nvs.clear();
            nvs.end();
        }

        void replaceRules(Rule rule) {
            clearRules();
            rule_store.push_back(rule);
            saveRules();
        }

        void replaceRules(ps_vector<Rule> rules) {
            clearRules();
            rule_store = rules;
            saveRules();
        }

        void replaceRules(size_t number, Rule rule) {
            rule_store.at(number) = rule;
            saveRules();
        }

        void appendRule(Rule rule) {
            rule_store.push_back(rule);
            saveRules();
        }

        void appendRule(ps_vector<Rule> rules) {
            for (size_t i = 0; i < rules.size(); i++) {
                rule_store.push_back(rules.at(i));
            }

            saveRules();
        }
};

class TagSearch {
    private:
    ps_vector<ps_string> class_tags;
    ps_string _nvs_path;

    void loadTags() {
        Preferences nvs;
        nvs.begin(_nvs_path.c_str());
        std::ostringstream path;

        size_t tag_count = nvs.getUInt("/count");

        for (size_t i = 0; i < tag_count; i++) {
            path << "/tag/" << i;
            ps_string tag = nvs.getString(path.str().c_str()).c_str();
            class_tags.push_back(tag);
            path.clear();
        }

        nvs.end();
    }

    void saveTags() {
        Preferences nvs;
        nvs.begin(_nvs_path.c_str());
        std::ostringstream path;

        nvs.putUInt("/count", class_tags.size());

        for (size_t i = 0; i < class_tags.size(); i++) {
            path << "/tag/" << i;
            nvs.putString(path.str().c_str(), class_tags.at(i).c_str());
            path.clear();
        }

        nvs.end();
    }

    public:
    TagSearch(const std::vector<std::string>& tag_list, const ps_string& nvs_path) {
        clearTags();

        ps_string temp;
        for (size_t i = 0; i < tag_list.size(); i++) { // Copy tag list to PSRAM.
            temp <<= tag_list.at(i);
            class_tags.push_back(temp);
            temp.clear();
        }

        saveTags();
    }

    TagSearch(const ps_vector<ps_string>& tag_list, const ps_string& nvs_path) {
        clearTags();
        class_tags = tag_list;
        saveTags();
    }

    TagSearch(const ps_string& nvs_path) {
        loadTags();
    }

    const ps_vector<ps_string>& getTags() {
        return class_tags;
    }

    void clearTags() {
        Preferences nvs;
        nvs.begin(_nvs_path.c_str());

        class_tags.clear();
        nvs.clear();

        nvs.end();
    }

    void appendTag(const ps_string& tag) {
        class_tags.push_back(tag);
        saveTags();
    }

    void appendTag(const ps_vector<ps_string>& tags) {
        for (size_t i = 0; i < tags.size(); i++) {
            class_tags.push_back(tags.at(i));
        }
        saveTags();
    }

    void replaceTag(const ps_string& tag) {
        clearTags();
        class_tags.push_back(tag);
        saveTags();
    }

    void replaceTag(const ps_vector<ps_string>& tags) {
        clearTags();
        class_tags = tags;
        saveTags();
    }

    void replaceTag(ps_string& tag, size_t loc) {
        class_tags.at(loc) = tag;
        saveTags();
    }

    /**
     * @brief Check whether a minimum number of tags match.
     * @param tag_list The list of tags to compare.
     * @param n The minimum number of matching tags before a true is returned.
     * @returns True if matching tags >= n, else false.
    */

    const bool tagMinQuantifierSearch(const ps_vector<ps_string>& tag_list, const size_t n) const {
        if ((class_tags.size() < n) || (tag_list.size() < n)) return false; // Not enough tags for a true outcome.

        size_t matches = 0;
        for (size_t src_it = 0; src_it < tag_list.size(); src_it++) {
            ps_string cur_str = tag_list.at(src_it);

            for (size_t class_it = 0; class_it < class_tags.size(); class_it++) {
                if (cur_str == class_tags.at(class_it)) {
                    matches++;
                    if (matches >= n) return true; // Stop searching after enough matches are found.
                    break;
                }
            }

        }

        return false;
    }

    /**
     * @brief Checks whether the provided tag is part of the modules tag list.
     * @return True if the tag is found in the module tag list, else false.
    */
    const bool tagSubsetComparison(const ps_string& tag) const {
        for (size_t i = 0; i < class_tags.size(); i++) {
            if (tag == class_tags.at(i)) return true;
        }

        return false;
    }

    /**
     * @brief Checks whether any of the tags provided are in the module tag list.
     * @return True if any tag is common between tag_list and the module tag list, else false.
    */
    const bool tagSubsetComparison(const ps_vector<ps_string>& tag_list) const {
        return tagMinQuantifierSearch(tag_list, 1);
    }

    /**
     * @brief Checks whether the provided tag_list matches exactly to the modules tag list. Element order does not matter.
     * @return True if the two vectors match, else false.
    */
    const bool tagEqualityComparison(const ps_vector<ps_string>& tag_list) const {
        return tagMinQuantifierSearch(tag_list, class_tags.size());
    }

};

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