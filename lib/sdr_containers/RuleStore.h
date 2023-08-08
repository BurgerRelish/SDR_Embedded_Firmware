#pragma once

#ifndef RULE_STORE_H
#define RULE_STORE_H

#include <stdint.h>
#include <string>
#include <iostream>
#include <sstream>
#include <ArduinoJson.h>

#include "../data_containers/ps_string.h"
#include "../data_containers/ps_vector.h"

struct Rule {
    int priority;
    ps_string expression;
    ps_string command;
};

class RuleStore {
    private:
        ps_vector<Rule> rule_store;
        ps_string _nvs_tag;

        void loadRules() {     
        }

        void saveRules() {
        }
    
    public:
        RuleStore(std::vector<Rule> rule_list) {
            rule_store <<= rule_list;
        }

        void saveRules(JsonArray& rule_array) {
            rule_array.clear();
            for (auto rule : rule_store) {
                auto rule_object = rule_array.createNestedObject();

                rule_object["pr"] = rule.priority;
                rule_object["exp"] = rule.expression.c_str();
                rule_object["cmd"] = rule.command.c_str();
            }
        }

        const ps_vector<Rule>& getRules() {
            return rule_store;
        }

        void clearRules() {
            rule_store.clear();

            // Preferences nvs;
            // nvs.begin(_nvs_tag.c_str());
            // nvs.clear();
            // nvs.end();
        }

        void replaceRules(Rule rule) {
            clearRules();
            rule_store.push_back(rule);
            //saveRules();
        }

        void replaceRules(ps_vector<Rule> rules) {
            clearRules();
            rule_store = rules;
            //saveRules();
        }

        void replaceRules(size_t number, Rule rule) {
            rule_store.at(number) = rule;
            //saveRules();
        }

        void appendRule(Rule rule) {
            rule_store.push_back(rule);
            //saveRules();
        }

        void appendRule(ps_vector<Rule> rules) {
            for (size_t i = 0; i < rules.size(); i++) {
                rule_store.push_back(rules.at(i));
            }

            //saveRules();
        }
};

#endif