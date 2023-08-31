// #include "VariableLookup.h"

// // Explicit instantiation for Module
// template class VariableLookup<Module>;

// // Explicit instantiation for SDRUnit
// template class VariableLookup<SDRUnit>;

// template <class BaseClass>
// const double VariableLookup<BaseClass>::getDouble(Token& token) {
//     double retval = false;

//     switch(token.type) {
//         case IDENTIFIER:
//             //ESP_LOGD(TAG_RULE_ENGINE, "Getting double value of IDENTIFIER token with lexeme: \'%s\'", token.lexeme.c_str());
//             switch(getVarType(token)) {
//                 case DOUBLE:
//                     retrieveVar(token.lexeme, retval);
//                     break;
//                 case INT:
//                     int val;
//                     retrieveVar(token.lexeme, val);
//                     retval = (double) val;
//                     break;
//                 case BOOL:
//                     bool bool_val;
//                     retrieveVar(token.lexeme, bool_val);
//                     retval = (double) bool_val;
//                     break;
//                 case UINT64_T:
//                     uint64_t uint64_val;
//                     retrieveVar(token.lexeme, uint64_val);
//                     retval = (double) val;
//                     break;
//                 default:
//                     throw std::invalid_argument("Cannot get double of this token type.");
//                 break;
//             }
//         break;
//         case NUMERIC_LITERAL:
//         //ESP_LOGD(TAG_RULE_ENGINE, "Getting double value of NUMERIC_LITERAL token with lexeme: \'%s\'", token.lexeme.c_str());
//         retval = toDouble(token.lexeme);
//         break;
//     }

//     return retval;
// }

// template <class BaseClass>
// const bool VariableLookup<BaseClass>::getBool(Token& token) {
//     bool retval = false;

//     switch(token.type) {
//         case IDENTIFIER:
//         //ESP_LOGD(TAG_RULE_ENGINE, "Getting bool value of IDENTIFIER token with lexeme: \'%s\'", token.lexeme.c_str());
//             switch(getVarType(token)) {
//                 case DOUBLE:
//                     double val;
//                     retrieveVar(token.lexeme, val);
//                     retval = (bool) val;
//                     break;
//                 case INT:
//                     int int_val;
//                     retrieveVar(token.lexeme, val);
//                     retval = (bool) int_val;
//                     break;
//                 case BOOL:
//                     retrieveVar(token.lexeme, retval);
//                     break;
//                 case UINT64_T:
//                     uint64_t uint64_val;
//                     retrieveVar(token.lexeme, val);
//                     retval = (bool) uint64_val;
//                     break;
//                 default:
//                     throw std::invalid_argument("Cannot get bool of this token type.");
//                 break;
//             }
//         break;
//         case NUMERIC_LITERAL:
//        // ESP_LOGD(TAG_RULE_ENGINE, "Getting bool value of NUMERIC_LITERAL token with lexeme: \'%s\'", token.lexeme.c_str());
//         retval = (bool) toDouble(token.lexeme);
//         break;
//     }

//     return retval;
// }

// template <class BaseClass>
// const int VariableLookup<BaseClass>::getInt(Token& token) {
//     int retval = false;

//     switch(token.type) {
//         case IDENTIFIER:
//             switch(getVarType(token)) {
//                 case DOUBLE:
//                     double val;
//                     retrieveVar(token.lexeme, val);
//                     retval = (int) val;
//                     break;
//                 case INT:
//                     retrieveVar(token.lexeme, retval);
//                     break;
//                 case BOOL:
//                     bool bool_val;
//                     retrieveVar(token.lexeme, val);
//                     retval = (int) bool_val;
//                     break;
//                 case UINT64_T:
//                     uint64_t uint64_val;
//                     retrieveVar(token.lexeme, val);
//                     retval = (int) uint64_val;
//                     break;
//                 default:
//                     throw std::invalid_argument("Cannot get int of this token type.");
//                 break;
//             }
//         break;
//         case NUMERIC_LITERAL:
//         retval = (int) toDouble(token.lexeme);
//         break;
//     }

//     return retval;
// }

// template <class BaseClass>
// const uint64_t VariableLookup<BaseClass>::getUint64(Token& token) {
//     uint64_t retval;

//     switch(token.type) {
//         case IDENTIFIER:
//             switch(getVarType(token)) {
//                 case DOUBLE:
//                     double val;
//                     retrieveVar(token.lexeme, val);
//                     retval = (uint64_t) val;
//                     break;
//                 case INT:
//                     int int_val;
//                     retrieveVar(token.lexeme, val);
//                     retval = (uint64_t) int_val;
//                     break;
//                 case BOOL:
//                     bool bool_val;
//                     retrieveVar(token.lexeme, val);
//                     retval = (uint64_t) bool_val;
//                     break;
//                 case UINT64_T:
//                     retrieveVar(token.lexeme, retval);
//                     break;
//                 default:
//                     throw std::invalid_argument("Cannot get bool of this token type.");
//                 break;
//             }
//         break;
//         case NUMERIC_LITERAL:
//         std::string val_str;
//         val_str <<= token.lexeme;

//         std::istringstream iss(val_str);

//         iss >> retval;
//         break;
//     }

//     return retval;
// }

// template <class BaseClass>
// const ps::string VariableLookup<BaseClass>::getString(Token& token) {
//     ps::string retval;

//     if (token.type == STRING_LITERAL) return token.lexeme;
//     if (token.type != IDENTIFIER) throw std::invalid_argument("Cannot get ps::string of this token type.");

//     retrieveVar(token.lexeme, retval);

//     return retval;
// }

// template <class BaseClass>
// double VariableLookup<BaseClass>::toDouble(ps::string& str) {
//     std::string string;
//     string <<= str;

//     return std::atof(string.c_str());
// }

// template <class BaseClass>
// const VarType&  VariableLookup<BaseClass>::getVarType(const Token& search_token) {
//     std::string string;
//     string <<= search_token.lexeme;

//     auto result = vartype_lookup.find(string);

//     if (result == vartype_lookup.end()) {
//         return vartype_lookup.find("INVALID_VAR") -> second;
//     }

//     return result -> second;
// }

// template <class BaseClass>
// template <typename U, typename std::enable_if<std::is_same<U, Module>::value, int>::type>
// void VariableLookup<BaseClass>::retrieveVar(const ps::string& var, double& val) {
//     std::string var_name;
//     var_name <<= var;

//     static const std::unordered_map<std::string, std::function<double()>> varMap = {
//         {TOTAL_ACTIVE_POWER, [this](){ return unit->totalActivePower(); }},
//         {TOTAL_REACTIVE_POWER, [this](){ return unit->totalReactivePower(); }},
//         {TOTAL_APPARENT_POWER, [this](){ return unit->totalApparentPower(); }},
//         {ACTIVE_POWER, [this](){ return module->latestReading().active_power; }},
//         {REACTIVE_POWER, [this](){ return module->latestReading().reactive_power; }},
//         {APPARENT_POWER, [this](){ return module->latestReading().apparent_power; }},
//         {VOLTAGE, [this](){ return module->latestReading().voltage; }},
//         {FREQUENCY, [this](){ return module->latestReading().frequency; }},
//         {POWER_FACTOR, [this](){ return module->latestReading().power_factor; }}
//     };

    
//     auto it = varMap.find(var_name);
//     if (it != varMap.end()) {
//         val = it->second();
//     } else {
//         throw std::invalid_argument("Invalid variable of type \'double\' requested.");
//     }
// }

// template <class BaseClass>
// template <typename U, typename std::enable_if<std::is_same<U, SDRUnit>::value, int>::type>
// void VariableLookup<BaseClass>::retrieveVar(const ps::string& var, double& val) {
//     std::string var_name;
//     var_name <<= var;

//     static const std::unordered_map<std::string, std::function<double()>> varMap = {
//         {TOTAL_ACTIVE_POWER, [this](){ return unit->totalActivePower(); }},
//         {TOTAL_REACTIVE_POWER, [this](){ return unit->totalReactivePower(); }},
//         {TOTAL_APPARENT_POWER, [this](){ return unit->totalApparentPower(); }}
//     };

    
//     auto it = varMap.find(var_name);
//     if (it != varMap.end()) {
//         val = it->second();
//     } else {
//         throw std::invalid_argument("Invalid variable of type \'double\' requested.");
//     }
// }

// template <class BaseClass>
// template <typename U, typename std::enable_if<std::is_same<U, Module>::value, int>::type>
// void VariableLookup<BaseClass>::retrieveVar(const ps::string& var, bool& val) {
//     std::string var_name;
//     var_name <<= var;

//     static const std::unordered_map<std::string, std::function<bool()>> varMap = {
//         {SWITCH_STATUS, [this](){ return module->status(); }},
//         {POWER_STATUS, [this](){ return unit->powerStatus(); }}
//     };

//     auto it = varMap.find(var_name);
//     if (it != varMap.end()) {
//         val = it->second();
//     } else {
//         throw std::invalid_argument("Invalid variable of type \'bool\' requested.");
//     }

//     return;
// }

// template <class BaseClass>
// template <typename U, typename std::enable_if<std::is_same<U, SDRUnit>::value, int>::type>
// void VariableLookup<BaseClass>::retrieveVar(const ps::string& var, bool& val) {
//     std::string var_name;
//     var_name <<= var;

//     static const std::unordered_map<std::string, std::function<bool()>> varMap = {
//         {POWER_STATUS, [this](){ return unit->powerStatus(); }}
//     };

//     auto it = varMap.find(var_name);
//     if (it != varMap.end()) {
//         val = it->second();
//     } else {
//         throw std::invalid_argument("Invalid variable of type \'bool\' requested.");
//     }

//     return;
// }

// template <class BaseClass>
// template <typename U, typename std::enable_if<std::is_same<U, Module>::value, int>::type>
// void VariableLookup<BaseClass>::retrieveVar(const ps::string& var, uint64_t& val) {
//     std::string var_name;
//     var_name <<= var;

//     static const std::unordered_map<std::string, std::function<uint64_t()>> varMap = {
//         {CURRENT_TIME, [this](){ return (uint64_t) time(nullptr); }},
//         {SWITCH_TIME, [this](){ return unit->moduleCount(); }}
//     };

//     auto it = varMap.find(var_name);
//     if (it != varMap.end()) {
//         val = it->second();
//     } else throw std::invalid_argument("Invalid variable of type \'uint64_t\' requested.");

//     return;
// }

// template <class BaseClass>
// template <typename U, typename std::enable_if<std::is_same<U, SDRUnit>::value, int>::type>
// void VariableLookup<BaseClass>::retrieveVar(const ps::string& var, uint64_t& val) {
//     std::string var_name;
//     var_name <<= var;

//     static const std::unordered_map<std::string, std::function<uint64_t()>> varMap = {
//         {CURRENT_TIME, [this](){ return (uint64_t) time(nullptr); }},
//     };

//     auto it = varMap.find(var_name);
//     if (it != varMap.end()) {
//         val = it->second();
//     } else throw std::invalid_argument("Invalid variable of type \'uint64_t\' requested.");

//     return;
// }

// template <class BaseClass>
// template <typename U, typename std::enable_if<std::is_same<U, Module>::value, int>::type>
// void VariableLookup<BaseClass>::retrieveVar(const ps::string& var, ps::string& val) {
//     if (var == MODULE_ID) {
//         val = module->id();
//     } else if (var == UNIT_ID) {
//         val = unit->id();
//     } else throw std::invalid_argument("Invalid variable of type \'ps::string\' requested.");

//     return;
// }

// template <class BaseClass>
// template <typename U, typename std::enable_if<std::is_same<U, SDRUnit>::value, int>::type>
// void VariableLookup<BaseClass>::retrieveVar(const ps::string& var, ps::string& val) {
//     if (var == UNIT_ID) {
//         val = unit->id();
//     } else throw std::invalid_argument("Invalid variable of type \'ps::string\' requested.");

//     return;
// }


