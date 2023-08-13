#pragma once

#ifndef RULE_ENGINE_H
#define RULE_ENGINE_H

#include <ps_stl.h>

#include "src/Language.h"
#include "Semantics.h"
#include "src/Rule.h"

namespace re {
struct RuleCompare {
    bool operator()(const std::shared_ptr<Rule>& a, const std::shared_ptr<Rule>& b) {
                return (a -> priority < b -> priority);
    }
};

class RuleEngine : public VariableStorage {
    private:
    std::shared_ptr<VariableStorage> variables;
    std::shared_ptr<FunctionStorage> functions;

    ps::priority_queue<std::shared_ptr<Rule>, RuleCompare> rule_queue;

    uint64_t last_time = 0;

    void load_rule_engine_vars();

    public:
    /**
     * @brief Construct a new Rule Engine object.
     * 
     * @param variable_store Variable storage.
     * @param function_store Global function storage.
     */
    RuleEngine(std::shared_ptr<FunctionStorage>& function_store) :
    functions(function_store), variables(VariableStorage::shared_from_this())
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
        rule_queue.push(ps::make_shared<Rule>(rule_priority, expression_str, command_str, variables, functions));
    }

    /**
     * @brief Adds a rule with the given priority to the evaluation queue.
     * 
     * @param rule tuple of rule (priority, expression, command)
     */
    void add_rule(std::tuple<int, ps::string, ps::string> rule) {
        rule_queue.push(ps::make_shared<Rule>(std::get<0>(rule), std::get<1>(rule), std::get<2>(rule), variables, functions));
    }


    /**
     * @brief Adds a rule with the given priority to the evaluation queue.
     * 
     * @param rules vector of tuple of rule (priority, expression, command)
     */
    void add_rule(ps::vector<std::tuple<int, ps::string, ps::string>> rules) {
        for (auto& rule : rules) {
            rule_queue.push(ps::make_shared<Rule>(std::get<0>(rule), std::get<1>(rule), std::get<2>(rule), variables, functions));
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
        auto exec = Executor(command_str, functions, variables);
        last_time = variables->get_var<uint64_t>(CURRENT_TIME);
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
        auto eval = Expression(expression_str, variables);
        if (eval.evaluate()) {
            auto exec = Executor(command_str, functions, variables);
            last_time = variables->get_var<uint64_t>(CURRENT_TIME);
            return exec.execute();
        }
        return false;
    }

    /**
     * @brief Evaluates all rules in the rule engine, and executes them if they evaluate true.
     * 
     */
    void reason() {
        auto rules = rule_queue;
        while (!rules.empty()) {
            rules.top() -> reason();
            rules.pop();
        }

        last_time = variables->get_var<uint64_t>(CURRENT_TIME);
    }
};

}

#endif