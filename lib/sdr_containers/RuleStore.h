#pragma once

#ifndef RULE_STORE_H
#define RULE_STORE_H

#include <Preferences.h>
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

#endif