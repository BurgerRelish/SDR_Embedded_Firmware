#pragma once

#ifndef RULE_ENGINE_BASE_H
#define RULE_ENGINE_BASE_H

#include <ArduinoJson.h>
#include <ps_stl.h>

#include "JSONFields.h"

#include "RuleEngine.h"

namespace re {

class RuleEngineBase : public RuleEngine {
    private:
    ps::vector<ps::string> class_tags;
    ps::vector<std::tuple<int, ps::string, ps::string>> rules;

    public:
    RuleEngineBase(const ps::string& tag_array_name, std::shared_ptr<FunctionStorage>& function_store) : RuleEngine(function_store) {
        RuleEngine::set_var(VAR_ARRAY, tag_array_name, [this](){return this -> class_tags;});
    }  

    RuleEngineBase(const ps::string& tag_array_name, std::shared_ptr<FunctionStorage>& function_store, ps::vector<ps::string>& tag_list) : RuleEngine(function_store), class_tags(tag_list) {
        RuleEngine::set_var(VAR_ARRAY, tag_array_name, [this](){return this -> class_tags;});
    }

    RuleEngineBase(const ps::string& tag_array_name, std::shared_ptr<FunctionStorage>& function_store, ps::vector<ps::string>& tag_list, ps::vector<std::tuple<int, ps::string, ps::string>> rule_list) : RuleEngine(function_store), class_tags(tag_list), rules(rule_list) {
        RuleEngine::set_var(VAR_ARRAY, tag_array_name, [this](){return this -> class_tags;});
    }

    ps::vector<std::tuple<int, ps::string, ps::string>>& get_rules() {
        return rules;
    }

    void clear_rules() override {
        RuleEngine::clear_rules();
        rules.clear();
    }

    void add_rule(int priority, ps::string& expression, ps::string& command) {
        add_rule(std::make_tuple(priority, expression, command));
    }

    void add_rule(std::tuple<int, ps::string, ps::string> new_rule){
        rules.push_back(new_rule);
        RuleEngine::add_rule(new_rule);
    }

    void add_rule(ps::vector<std::tuple<int, ps::string, ps::string>> new_rules) {
        for (auto& rule : new_rules) {
            rules.push_back(rule);
            RuleEngine::add_rule(rule);
        } 
    }

    void replace_rules(std::tuple<int, ps::string, ps::string> new_rule) {
        clear_rules();
        add_rule(new_rule);
    }

    void replace_rules(ps::vector<std::tuple<int, ps::string, ps::string>> new_rules) {
        clear_rules();
        add_rule(new_rules);
    }

    /**
     * @brief Clear the tag list.
     * 
     */
    void clear_tags() {
        class_tags.clear();
    }
    /**
     * @brief Get a read write reference to the class tags list.
     * 
     * @return ps::vector<ps::string>& 
     */
    ps::vector<ps::string>& get_tags() {
        return class_tags;
    }

    /**
     * @brief Get a read only reference to the class tags list.
     * 
     * @return const ps::vector<ps::string>& 
     */
    const ps::vector<ps::string>& get_ctags() {
        return class_tags;
    }

    /**
     * @brief Add tag(s) to the tag list.
     * 
     * @param tags 
     */
    void add_tag(ps::vector<ps::string> tags) {
        for (auto& tag : tags) {
            class_tags.push_back(tag);
        }
    }

    /**
     * @brief Add tag(s) to the tag list.
     * 
     * @param tags 
     */
    void add_tag(ps::string tag) {
        class_tags.push_back(tag);
    }


    /**
     * @brief Clear the tag list and add the new tags to it.
     * 
     * @param tags 
     */
    void replace_tag(ps::vector<ps::string> tags) {
        class_tags = tags;
    }


    /**
     * @brief Clear the tag list and add the new tags to it.
     * 
     * @param tags 
     */
    void replace_tag(ps::string tag) {
        class_tags.clear();
        class_tags.push_back(tag);
    }

    void load_rule_engine(JsonObject& obj) {
        auto rule_arr = obj[JSON_RULES].as<JsonArray>();
        
        for (auto rule : rule_arr) {
            auto tp = std::make_tuple(rule[JSON_PRIORITY].as<int>(), rule[JSON_EXPRESSION].as<ps::string>(), rule[JSON_COMMAND].as<ps::string>());
            add_rule(tp);
        }

        JsonArray tag_arr = obj[JSON_TAGS].as<JsonArray>();
        for (auto tag : tag_arr) {
            class_tags.push_back(tag.as<ps::string>());
        }
    }

    void save_rule_engine(JsonObject& obj) {
        JsonArray rule_arr = obj.createNestedArray(JSON_RULES);
        for (auto& rule : rules) {
            auto rule_obj = rule_arr.createNestedObject();
            rule_obj[JSON_PRIORITY] = std::get<0>(rule);
            rule_obj[JSON_EXPRESSION] = std::get<1>(rule).c_str();
            rule_obj[JSON_COMMAND] = std::get<2>(rule).c_str();
        }

        JsonArray tag_arr = obj.createNestedArray(JSON_TAGS);
        for (auto& tag : class_tags) {
            tag_arr.add(tag.c_str());
        }
    }
};

}

#endif