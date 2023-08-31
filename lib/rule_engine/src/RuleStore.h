#pragma once

#ifndef RULE_STORE_H
#define RULE_STORE_H

#include <stdint.h>
#include <string>
#include <iostream>
#include <sstream>
#include <ArduinoJson.h>

#include <ps_stl.h>


struct Rule {
    int priority;
    ps::string expression;
    ps::string command;
};

class RuleStore {
    private:
        ps::vector<Rule> rule_store;
        ps::string _nvs_tag;

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

        const ps::vector<Rule>& getRules() {
            return rule_store;
        }

        void clearRules() {
            rule_store.clear();
        }

        void replaceRules(Rule rule) {
            clearRules();
            rule_store.push_back(rule);
        }

        void replaceRules(ps::vector<Rule> rules) {
            clearRules();
            rule_store = rules;
        }

        void replaceRules(size_t number, Rule rule) {
            rule_store.at(number) = rule;
        }

        void appendRule(Rule rule) {
            rule_store.push_back(rule);
        }

        void appendRule(ps::vector<Rule> rules) {
            for (size_t i = 0; i < rules.size(); i++) {
                rule_store.push_back(rules.at(i));
            }
        }
};

#endif