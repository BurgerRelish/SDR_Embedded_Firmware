#pragma once

#ifndef RULE_ENGINE_BASE_H
#define RULE_ENGINE_BASE_H

#include <ps_stl.h>

#include "../rule_engine/RuleEngine.h"

namespace re {
    class RuleEngineBase : public RuleEngine {
    private:
    ps::vector<ps::string> class_tags;
    ps::vector<std::tuple<int, ps::string, ps::string>> rules;

    public:
    RuleEngineBase(const ps::string& tag_array_name, std::shared_ptr<FunctionStorage>& function_store) : RuleEngine(function_store) {
        RuleEngine::set_var(VAR_ARRAY, tag_array_name, class_tags);
    }  

    RuleEngineBase(const ps::string& tag_array_name, std::shared_ptr<FunctionStorage>& function_store, ps::vector<ps::string>& tag_list) : RuleEngine(function_store), class_tags(tag_list) {
        RuleEngine::set_var(VAR_ARRAY, tag_array_name, class_tags);
    }

    RuleEngineBase(const ps::string& tag_array_name, std::shared_ptr<FunctionStorage>& function_store, ps::vector<ps::string>& tag_list, ps::vector<std::tuple<int, ps::string, ps::string>> rule_list) : RuleEngine(function_store), class_tags(tag_list), rules(rule_list) {
        RuleEngine::set_var(VAR_ARRAY, tag_array_name, class_tags);
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

};

}

#endif