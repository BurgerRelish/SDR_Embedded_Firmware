#pragma once

#ifndef RULE_ENGINE_H
#define RULE_ENGINE_H

#include <ps_stl.h>

#include "src/Language.h"
#include "src/Semantics.h"
#include "src/Rule.h"

namespace re {
struct RuleCompare {
    bool operator()(const std::shared_ptr<Rule>& a, const std::shared_ptr<Rule>& b) {
                return (a -> priority < b -> priority);
    }
};

class RuleEngine : public VariableStorage {
    private:
    std::shared_ptr<FunctionStorage> functions;

    ps::priority_queue<std::shared_ptr<Rule>, RuleCompare> rule_queue;

    uint64_t last_time = 0;

    void load_rule_engine_vars() {
        VariableStorage::set_var(VAR_UINT64_T, LAST_EXECUTION_TIME, std::function<uint64_t()>([this]() { return this->last_time; }));

        VariableStorage::set_var(VAR_UINT64_T, CURRENT_TIME, std::function<uint64_t()>([]() {
                    time_t now;
                    struct tm timeinfo;
                    if(!getLocalTime(&timeinfo)){
                        ESP_LOGE("Time", "Failed to get time.");
                        return static_cast<uint64_t>(0);
                    }
                    
                    return static_cast<uint64_t>(mktime(&timeinfo));
                }
            )
        );
    }

    void create_rule(int rule_priority, ps::string expression_str, ps::string command_str) {
        VariableStorage* vars = this;
        auto rule = ps::make_shared<Rule>(rule_priority, expression_str, command_str, vars, functions);
        
        ESP_LOGV("rule", "Created, adding to queue.");
        rule_queue.push(rule);
    }

    public:
    /**
     * @brief Construct a new Rule Engine object.
     * 
     * @param variable_store Variable storage.
     * @param function_store Global function storage.
     */
    RuleEngine(std::shared_ptr<FunctionStorage>& function_store) :
    functions(function_store)
    {
        load_rule_engine_vars();
    }

    /**
     * @brief Adds a rule with the given priority to the evaluation queue.
     * 
     * @param rule_priority Priority of the queue (Higher Values == Higher Priority)
     * @param expression_str Expression of rule.
     * @param command_str Commands of rule.
     */
    void add_rule(const int rule_priority, const ps::string& expression_str, const ps::string& command_str) {
        create_rule(rule_priority, expression_str, command_str);
    }

    /**
     * @brief Adds a rule with the given priority to the evaluation queue.
     * 
     * @param rule tuple of rule (priority, expression, command)
     */
    void add_rule(std::tuple<int, ps::string, ps::string> rule) {
        create_rule(std::get<0>(rule), std::get<1>(rule), std::get<2>(rule));
    }


    /**
     * @brief Adds a rule with the given priority to the evaluation queue.
     * 
     * @param rules vector of tuple of rule (priority, expression, command)
     */
    void add_rule(ps::vector<std::tuple<int, ps::string, ps::string>> rules) {
        for (auto& rule : rules) {
            create_rule(std::get<0>(rule), std::get<1>(rule), std::get<2>(rule));
        }
    }

    /**
     * @brief Clears all currently saved rules from the evaluator.
     * 
     */
    virtual void clear_rules() {
        while (!rule_queue.empty()) rule_queue.pop();
    }

    /**
     * @brief Directly executes the provided command string.
     * 
     * @param command_str 
     * @return true Command executed successfully.
     * @return false Command failed to execute.
     */
    bool execute(const ps::string& command_str) {
        auto exec = Executor(command_str, functions, this);
        last_time = VariableStorage::get_var<uint64_t>(CURRENT_TIME);
        return exec.execute();
    }

    /**
     * @brief Executes the command if the expression evaluates to true.
     * 
     * @param expression_str Expression to evaluate
     * @param command_str Command to execute if the expression is true.
     * @return true Expression was true and Command ran successfully.
     * @return false Either the Expression evaluated false, or the command execution failed.
     */
    bool execute_if(const ps::string& expression_str, const ps::string& command_str) {
        auto eval = Expression(expression_str, this);
        if (eval.evaluate()) {
            auto exec = Executor(command_str, functions, this);
            last_time = VariableStorage::get_var<uint64_t>(CURRENT_TIME);
            return exec.execute();
        }
        return false;
    }

    /**
     * @brief Evaluates all rules in the rule engine, and executes the highest priority to evaluate true.
     * 
     */
    void reason() {
        auto rules = rule_queue;
        while (!rules.empty()) {
            if(rules.top() -> reason()) {
                last_time = VariableStorage::get_var<uint64_t>(CURRENT_TIME);
                return;
            }
            rules.pop();
        }
    }
};

}

#endif