#pragma once

#ifndef FUNCTION_H
#define FUNCTION_H

#include "Lexer.h"
#include "VariableStorage.h"
#include "CommandSeparator.h"

namespace re {

class FunctionStorage {
    private:        
        ps::unordered_map<ps::string, std::function<bool(ps::vector<Token>&, std::shared_ptr<VariableStorage>&)>> function_map;

    public:
        void add(const ps::string& identifier, std::function<bool(ps::vector<Token>&, std::shared_ptr<VariableStorage>&)> function) {
            function_map[identifier] = function;
        }

        bool execute(const ps::string& identifier, ps::vector<Token>& args, std::shared_ptr<VariableStorage>& vars) {
            auto lambda = function_map.find(identifier);

            if (lambda == function_map.end()) {
                ESP_LOGD("Map Search", "Invalid Function Identifier.");
                return false;
            }

            try {
                return lambda -> second(args, vars);
            } catch (...) {
                ESP_LOGD("Exception", "Lambda execution.");
                return false;
            }

            return true;
        }
};

class Executor {
    private:
    std::shared_ptr<FunctionStorage> fn_store;
    std::shared_ptr<VariableStorage>& var_store;
    ps::vector<std::pair<ps::string, ps::vector<Token>>> commands;

    public:
    Executor(ps::string command, std::shared_ptr<FunctionStorage>& functions, std::shared_ptr<VariableStorage>& variables) : fn_store(functions), var_store(variables) {
        commands = CommandSeparator::separate(command);
    }

    bool execute() {
        bool ret = true;
        for (auto& command : commands) {
            try {
                auto result = fn_store -> execute(
                    command.first,
                    command.second,
                    var_store
                );

                if (result != true) {
                    ESP_LOGE("Fail", "Command %s failed.", command.first.c_str());
                }
                ret = ret && result;
            } catch (...) {
                ESP_LOGE("Except", "Command %s failed.", command.first.c_str());
            }
        }

        return ret;
    }
};

}

#endif