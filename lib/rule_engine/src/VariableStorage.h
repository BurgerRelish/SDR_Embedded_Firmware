#pragma once

#ifndef VARIABLE_STORAGE_H
#define VARIABLE_STORAGE_H
#include <Arduino.h>
#include <unordered_map>
#include <any>
#include <string>
#include <memory>
#include <sstream>
#include <type_traits>
#include <functional>

#include "../sdr_containers/SDRUnit.h"
#include "../sdr_containers/SDRModule.h"
#include "../ps_stl/ps_stl.h"

#include "../rule_engine/Semantics.h"

#include "var_cast.h"

namespace re {

class VariableStorage;

enum VariableType {
    VAR_INT,
    VAR_BOOL,
    VAR_DOUBLE,
    VAR_STRING,
    VAR_UINT64_T,
    VAR_ARRAY,
    VAR_CLASS,
    VAR_UNKNOWN
};


class VariableStorage {
    private:
    ps::unordered_map<ps::string, std::pair<VariableType, std::any>> storage;

    public:
    VariableStorage() {}
    ~VariableStorage() {}

    VariableType type(ps::string identifier);
    template <typename T>
    void set(VariableType type, ps::string identifier, const T value) {
        ESP_LOGV("Set", "Id: %s, Tp: %d", identifier.c_str(), type);
        storage.insert(std::make_pair(
            identifier,
            std::make_pair(
                type, 
                std::any(value)
            )
        ));
    }

    /**
     * @brief Fetches the value of an identifier string. Searches through own unordered_maps first. 
     * If no match is found, it attempts to cast the identifier string into the requested value.
     * 
     * @tparam T - Type of variable to retrieve.
     * @param identifier 
     * @return T - The value of the variable with semantics matching the identifier, or the cast value of the identifier.
     */
    template <typename T>
    T get(const ps::string& identifier) {
        auto var = storage.find(identifier.c_str());
        if (var != storage.end()) {
            try { /* Execute lambda */
                switch (var -> second.first) {
                        case (VAR_INT): {
                            ESP_LOGV("Int", "Found: %s", identifier.c_str());
                            auto fn = std::any_cast<std::function<int(void)> >(var->second.second);
                            auto ret = fn();
                            ESP_LOGV("Int", "Got: %d", ret);
                            return var_cast<int>(ret);
                            
                            break;
                        }
                        case (VAR_BOOL): {
                            ESP_LOGV("Bool", "Found: %s", identifier.c_str());
                            auto fn = std::any_cast<std::function<bool(void)> >(var->second.second);
                            auto ret = fn();
                            ESP_LOGV("Bool", "Got: %u", ret);
                            return var_cast<bool>(ret);
                            break;
                        }
                        case (VAR_DOUBLE): {
                            auto fn = std::any_cast<std::function<double(void)> >(var->second.second);
                            auto ret = fn();
                            ESP_LOGV("Dbl", "Got: %f", ret);
                            return var_cast<double>(ret);
                            break;
                        }
                        case (VAR_STRING): {
                            auto fn = std::any_cast<std::function<ps::string(void)> >(var->second.second);
                            auto ret = fn();
                            ESP_LOGV("Str", "Got: %s", ret.c_str());
                            return (T) var_cast<ps::string>(ret);
                            break;
                        }
                        case (VAR_UINT64_T): {
                            auto fn = std::any_cast<std::function<uint64_t(void)> >(var->second.second);
                            auto ret = fn();
                            ESP_LOGV("uint64", "Got: %u", ret);
                            return (T) var_cast<uint64_t>(ret);
                            break;
                        }
                        case (VAR_ARRAY): {
                            auto fn = std::any_cast<std::function<ps::vector<ps::string>(void)> >(var->second.second);
                            auto ret = var_cast<ps::vector<ps::string>>(fn());
                            ESP_LOGV("arr", "Got.");
                            return ret;
                            break;
                        }
                        default: throw;
                }
            } catch (const std::bad_any_cast& e) {
                try {
                    return std::any_cast<T> (var -> second.second); // Otherwise try cast to return val.
                } catch (const std::bad_any_cast& e) {
                    ESP_LOGE("Cast", "Fail: %e", e.what());
                }
            } catch (...) {}

        } else {
            ESP_LOGV("find", "Unknown variable name: \'%s\'.", identifier.c_str());
        }

        /* If all else fails, try cast the identifier to the return value. */
        try {
            auto ret = var_cast<ps::string> (identifier);
            return ret;
        } catch (...) {
            ESP_LOGE("Cast", "Failed to reinterpret.");
        }
        
        return T();
    }

    /**
     * @brief Method to get std::shared_ptr
     * 
     * @tparam T 
     * @param identifier 
     * @return T 
     */
    template <typename T>
    T get_direct(const ps::string& identifier) {
        auto var = storage.find(identifier);
        if (var != storage.end()) {
            try {
                return std::any_cast<T>(var->second.second);
            } catch (const std::bad_any_cast& e) {
                ESP_LOGE("Cast", "Fail: %e", e.what());
            }
        }

        return T(nullptr);
    }
};

}



#endif