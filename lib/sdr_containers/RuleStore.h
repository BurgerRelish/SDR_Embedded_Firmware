#pragma once

#ifndef RULE_STORE_H
#define RULE_STORE_H

#include <stdint.h>
#include <string>
#include <iostream>
#include <sstream>

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
        RuleStore(const ps_string& nvs_tag): _nvs_tag(nvs_tag) {
            //loadRules();
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