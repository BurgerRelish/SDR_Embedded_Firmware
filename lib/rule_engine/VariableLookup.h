// #pragma once

// #ifndef VARIABLE_LOOKUP_H
// #define VARIABLE_LOOKUP_H
// #include <Arduino.h>
// #include <type_traits>
// #include <memory>

// #include "../sdr_containers/SDRModule.h"
// #include "../sdr_containers/SDRUnit.h"
// #include "Language.h"
// #include "../ps_stl/ps_stl.h"


// template <class BaseClass>
// class VariableLookup {
//     private:
//     /* SDR Unit Identifer Names */
//     template <typename U = BaseClass, typename std::enable_if<std::is_same<U, Module>::value, int>::type = 0>
//     const std::unordered_map<std::string, VarType> vartype_lookup {
//         /* Global Variables */
//         {TOTAL_ACTIVE_POWER, DOUBLE},
//         {TOTAL_REACTIVE_POWER, DOUBLE},
//         {TOTAL_APPARENT_POWER, DOUBLE},
//         {POWER_STATUS, BOOL},
//         {UNIT_ID, PS_STRING},
//         {MODULE_COUNT, INT},
//         {UNIT_TAG_LIST, ARRAY_VAR},
//         {CURRENT_TIME, UINT64_T},

//         /* Module Variables */
//         {ACTIVE_POWER, DOUBLE},
//         {REACTIVE_POWER, DOUBLE},
//         {APPARENT_POWER, DOUBLE},
//         {VOLTAGE, DOUBLE},
//         {FREQUENCY, DOUBLE},
//         {POWER_FACTOR, DOUBLE},
//         {SWITCH_TIME, BOOL},
//         {CIRCUIT_PRIORITY, INT},
//         {MODULE_ID, PS_STRING},
//         {MODULE_TAG_LIST, ARRAY_VAR},
//         {SWITCH_TIME, UINT64_T},
//         {SWITCH_STATUS, BOOL},
//         {"INVALID_VAR", INVALID_VAR}
//     };

//     template <typename U = BaseClass, typename std::enable_if<std::is_same<U, SDRUnit>::value, int>::type = 0>
//     const std::unordered_map<std::string, VarType> vartype_lookup {
//         /* Global Variables */
//         {TOTAL_ACTIVE_POWER, DOUBLE},
//         {TOTAL_REACTIVE_POWER, DOUBLE},
//         {TOTAL_APPARENT_POWER, DOUBLE},
//         {POWER_STATUS, BOOL},
//         {UNIT_ID, PS_STRING},
//         {MODULE_COUNT, INT},
//         {UNIT_TAG_LIST, ARRAY_VAR},
//         {CURRENT_TIME, UINT64_T}
//     };

//     /* Module Variable Lookup */
//     template <typename U = BaseClass, typename std::enable_if<std::is_same<U, Module>::value, int>::type = 0>
//     void retrieveVar(const ps::string& var, double& val);

//     template <typename U = BaseClass, typename std::enable_if<std::is_same<U, Module>::value, int>::type = 0>
//     void retrieveVar(const ps::string& var, bool& val);

//     template <typename U = BaseClass, typename std::enable_if<std::is_same<U, Module>::value, int>::type = 0>
//     void retrieveVar(const ps::string& var, int& val);

//     template <typename U = BaseClass, typename std::enable_if<std::is_same<U, Module>::value, int>::type = 0>
//     void retrieveVar(const ps::string& var, uint64_t& val);

//     template <typename U = BaseClass, typename std::enable_if<std::is_same<U, Module>::value, int>::type = 0>
//     void retrieveVar(const ps::string& var, ps::string& val);

//     /* Unit Variable Lookup */
//     template <typename U = BaseClass, typename std::enable_if<std::is_same<U, SDRUnit>::value, int>::type = 0>
//     void retrieveVar(const ps::string& var, double& val);

//     template <typename U = BaseClass, typename std::enable_if<std::is_same<U, SDRUnit>::value, int>::type = 0>
//     void retrieveVar(const ps::string& var, bool& val);

//     template <typename U = BaseClass, typename std::enable_if<std::is_same<U, SDRUnit>::value, int>::type = 0>
//     void retrieveVar(const ps::string& var, int& val);

//     template <typename U = BaseClass, typename std::enable_if<std::is_same<U, SDRUnit>::value, int>::type = 0>
//     void retrieveVar(const ps::string& var, uint64_t& val);

//     template <typename U = BaseClass, typename std::enable_if<std::is_same<U, SDRUnit>::value, int>::type = 0>
//     void retrieveVar(const ps::string& var, ps::string& val);

//     double toDouble(ps::string& str);
    
//     public:
//     std::shared_ptr<SDRUnit> unit;
//     std::shared_ptr<Module> module;

//     VariableLookup(std::shared_ptr<SDRUnit>& _unit) : unit(_unit), module(nullptr) {}
//     VariableLookup(std::shared_ptr<SDRUnit>& _unit, std::shared_ptr<Module>& _module) : unit(_unit), module(_module) {}

//     const VarType& getVarType(const Token& search_token);

//     const double getDouble(Token& token);
//     const bool getBool(Token& token);
//     const int getInt(Token& token);
//     const uint64_t getUint64(Token& token);
//     const ps::string getString(Token& token);
// };

// #endif