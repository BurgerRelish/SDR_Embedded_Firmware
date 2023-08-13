#pragma once

#ifndef VAR_CAST_H
#define VAR_CAST_H

#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <ps_stl.h>

template<typename T>
class var_cast {
public:
    T value;

    var_cast(const T& val) : value(val) {}

    operator bool() const {
        if constexpr (std::is_same_v<T, bool>) {
            return value;
        } else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, uint64_t>) {
            return static_cast<bool>(value);
        } else if constexpr (std::is_same_v<T, double>) {
            return value > 0.001;
        } else if constexpr (std::is_same_v<T, ps::string>) {
                ps::istringstream iss(value);
                int result;
                if (iss >> result) {
                    return result > 0.001;
                }
                return 0; // If not a number, return 0
        } else if constexpr (std::is_same_v<T, ps::vector<ps::string>>) {
            return !value.empty();
        } else {
            throw std::runtime_error("Conversion not supported.");
        }
    }

    operator int() const {
        if constexpr (std::is_same_v<T, int>) {
            return value;
        } else if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, uint64_t> || std::is_same_v<T, double>) {
            return static_cast<int>(value);
        } else if constexpr (std::is_same_v<T, ps::string>) {
                ps::istringstream iss(value);
                double result;
                if (iss >> result) {
                    return static_cast<int>(result);
                }
                return 0; // If not a number, return 0
        } else if constexpr (std::is_same_v<T, ps::vector<ps::string>>) {
            return static_cast<int>(value.size());
        } else {
            throw std::runtime_error("Conversion not supported.");
        }
    }

    operator double() const {
        if constexpr (std::is_same_v<T, double>) {
            return value;
        } else if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, uint64_t> || std::is_same_v<T, int>) {
            return static_cast<double>(value);
        } else if constexpr (std::is_same_v<T, ps::string>) {
                ps::istringstream iss(value);
                double result;
                if (iss >> result) {
                    return result;
                }
                return 0; // If not a number, return 0
        } else if constexpr (std::is_same_v<T, ps::vector<ps::string>>) {
            return static_cast<double>(value.size());
        } else {
            throw std::runtime_error("Conversion not supported.");
        }
    }

    operator uint64_t() const {
        if constexpr (std::is_same_v<T, uint64_t>) {
            return value;
        } else if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, int> || std::is_same_v<T, double>) {
            return static_cast<uint64_t>(value);
        } else if constexpr (std::is_same_v<T, ps::string>) {
                ps::istringstream iss(value);
                uint64_t result;
                if (iss >> result) {
                    return result;
                }
                return 0; // If not a number, return 0
        } else if constexpr (std::is_same_v<T, ps::vector<ps::string>>) {
            return static_cast<uint64_t>(value.size());
        } else {
            throw std::runtime_error("Conversion not supported.");
        }
    }

    operator ps::string() const {
        if constexpr (std::is_same_v<T, ps::vector<ps::string>>) {
            ps::stringstream ss;
            for (const auto& str : value) {
                ss << str << ",";
            }
            ps::string result = ss.str();
            if (!result.empty()) {
                result.pop_back(); // Remove the last comma
            }
            return result;
        } else if constexpr (std::is_same_v<T, ps::string>) {
            return value;
        } else if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, uint64_t> || std::is_same_v<T, double> || std::is_same_v<T, int>) {
            ps::stringstream ss;
            ss << value;
            return ss.str();
        } else {
            throw std::runtime_error("Conversion not supported.");
        }
    }

    operator ps::vector<ps::string>() const {
        if constexpr (std::is_same_v<T, ps::string>) {
            ps::vector<ps::string> result;
            result.push_back(value);
            return result;
        } else if constexpr (std::is_same_v<T, ps::vector<ps::string>>) {
            return value;
        } else if constexpr (std::is_same_v<T, bool> || std::is_same_v<T, uint64_t> || std::is_same_v<T, double> || std::is_same_v<T, int>) {
            ps::stringstream ss;
            ss << value;
            ps::vector<ps::string> result;
            result.push_back(ss.str());
            return result; 
        } else {
            throw std::runtime_error("Conversion not supported.");
        }
    }
};

#endif
