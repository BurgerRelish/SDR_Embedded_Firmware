#include "Evaluator.h"
#include <math.h>
#include <time.h>
#include <iostream>
#include <sstream>

#ifdef DEBUG_RULE_ENGINE
#include "esp_timer.h"
#include "esp32-hal-log.h"
#define TAG_RULE_ENGINE "RULE_ENGINE"
#endif

void Evaluator::generateRules(ps_queue<Rule>& rule_input) {
    Rule current_rule;
    LexedRule new_rule;
    ShuntingYard rpn_algorithm;

    while(!rule_input.empty()) {
        current_rule = rule_input.front();
        new_rule.priority = current_rule.priority;

        // Lex each rule and convert it to postfix notation, ready for evaluation. 
        ps_queue<Token> tokens = lexExpression(current_rule.expression);
        new_rule.expression = rpn_algorithm.apply(tokens);

        new_rule.commands = lexExpression(current_rule.command);

        rules.push_back(new_rule);
        rule_input.pop();
    }   
}

ps_queue<Token> Evaluator::evaluate() {
    #ifdef DEBUG_RULE_ENGINE
    uint64_t start_time = esp_timer_get_time();
    #endif
    ps_stack<LexedRule> rule_stack;
    int highest_priority = 0;

    // Evaluate all rules which have a priority greater than the last successful rule.
    for (size_t i = 0; i < rules.size(); i++) {
        if (rules.at(i).priority >= highest_priority) {
            if (evaluateRPN(rules.at(i).expression)) {
                rule_stack.push(rules.at(i));
                highest_priority = rules.at(i).priority;
            }
        }
    }

    ps_queue<Token> commands;

    // Only return the commands which resulted from the rules with the highest priority.
    while (!rule_stack.empty()) {
        if (rule_stack.top().priority == highest_priority) {
            while(!rule_stack.top().commands.empty()) { // Merge all command tokens into a single queue.
                commands.push(rule_stack.top().commands.front());
                rule_stack.top().commands.pop();
            }
        }
        rule_stack.pop();
    }

    #ifdef DEBUG_RULE_ENGINE
    uint64_t tot_time = (esp_timer_get_time() - start_time);
    
    ESP_LOGD(TAG_RULE_ENGINE, "\n==== Rule Evaluation Completed ====");
    log_printf("- Processing Time: %uus\n", tot_time);
    log_printf("- Successful Rules: %d\n", commands.size());
    ps_string debug;
    ps_queue<Token> debug_queue = commands;

    while(!debug_queue.empty()) {
        debug += debug_queue.front().lexeme;
        debug += " ";
        debug_queue.pop();
    }

    log_printf("- Tokens: \'%s\'\n", debug.c_str());
    log_printf("====================================\r\n");
    #endif

    return commands;
}

ps_queue<Token> Evaluator::lexExpression(ps_string& expr) {
    Lexer lexer(expr);
    return lexer.tokenize();
}

bool Evaluator::evaluateRPN(ps_queue<Token>& tokens) {
    ps_queue<Token> token_list = tokens;
    ps_stack<Token> token_stack;
    
    // Evaluate all commands in the expression.
    while (!token_list.empty()) {
        switch(token_list.front().type) {
            case IDENTIFIER:
            case STRING_LITERAL: // "Hello World!"
            case NUMERIC_LITERAL: // 1 1.201
            case ARRAY: // ]
                //ESP_LOGD(TAG_RULE_ENGINE, "Pushed literal to token stack.");
                token_stack.push(token_list.front()); // Push all literals and identifiers to the stack.
                break;
            case ARITHMETIC_OPERATOR: // + - / ^ %
            case BOOLEAN_OPERATOR: // && || !
            case COMPARISON_OPERATOR: // == != <= >= < >
                //ESP_LOGD(TAG_RULE_ENGINE, "Processing Operator.");
                evaluateOperator(token_stack, token_list.front()); // Apply the operator token.
                break;
            default:
                throw std::invalid_argument("Unrecognized operation.");
                break;
        }
        token_list.pop();
    }

    if(token_stack.size() != 1) throw std::invalid_argument("Unbalanced expression.");
    if(token_stack.top().type != NUMERIC_LITERAL) throw std::invalid_argument("Failed to evaluate expression.");

    return (bool) std::atof(token_stack.top().lexeme.c_str());
}

void Evaluator::evaluateOperator(ps_stack<Token>& tokens, Token& operator_token) {
    Token rhs = tokens.top();
    VarType rhs_type = getVarType(rhs);
    tokens.pop();

    Token lhs = tokens.top();
    VarType lhs_type = getVarType(lhs);
    tokens.pop();

    Token result;
    result.type = NUMERIC_LITERAL;
    std::string val_str;

    

    // Handle arrays and strings separately
    if ((lhs_type == PS_STRING && rhs.type == STRING_LITERAL) || (rhs_type == PS_STRING && lhs.type == STRING_LITERAL) || (lhs_type == PS_STRING && rhs_type == PS_STRING)) {
        val_str = std::to_string((double) applyStringComparison(lhs, rhs, operator_token));
        result.lexeme <<= val_str;
        tokens.push(result);
        return;
    } else if (lhs_type == ARRAY_VAR || rhs_type == ARRAY_VAR) {
        val_str = std::to_string((double) applyArrayComparison(lhs, rhs, operator_token));
        result.lexeme <<= val_str;
        tokens.push(result);
        return;
    }

    // Evaluate the operator and push its result to the output stack.
    switch (operator_token.type) {
        case BOOLEAN_OPERATOR:
            val_str = std::to_string((double) applyBooleanOperator(lhs, rhs, operator_token));
            result.lexeme <<= val_str;
            tokens.push(result);
            return;
        case ARITHMETIC_OPERATOR:
            val_str = std::to_string((double) applyArithmeticOperator(lhs, rhs, operator_token));
            result.lexeme <<= val_str;
            tokens.push(result);
            return;
        case COMPARISON_OPERATOR:
            val_str = std::to_string((double) applyComparisonOperator(lhs, rhs, operator_token));
            result.lexeme <<= val_str;
            tokens.push(result);
            return;
    }

    throw std::invalid_argument("Could not evaluate token.");
}

bool Evaluator::applyBooleanOperator(Token& lhs, Token& rhs, Token& operator_token) {
    bool lhs_val = getBool(lhs);
    bool rhs_val = getBool(rhs);

    #ifdef DEBUG_RULE_ENGINE
    ESP_LOGD(TAG_RULE_ENGINE, "\n==== Apply Boolean Operator ====");
    log_printf("- Operator: %s", operator_token.lexeme.c_str());
    log_printf("\n- LHS Lexeme: %s Value: %d", lhs.lexeme.c_str(), (int) lhs_val);
    log_printf("\n- RHS Lexeme: %s Value: %d", rhs.lexeme.c_str(), (int) rhs_val);
    #endif

    bool retval;

    if (operator_token.lexeme == BOOLEAN_AND) {
        retval = ((bool) lhs_val && (bool) rhs_val);
    } else if (operator_token.lexeme == BOOLEAN_OR) {
        retval = ((bool) lhs_val && (bool) rhs_val);
    } else if (operator_token.lexeme == BOOLEAN_NOT) {
        retval = (!(bool) lhs_val);
    } else throw std::invalid_argument("Invalid Boolean Operator");

    #ifdef DEBUG_RULE_ENGINE
    log_printf("\n- Outcome: %d", (int) retval);
    log_printf("\n====================================\r\n\n");
    #endif

    return retval;
}

double Evaluator::applyArithmeticOperator(Token& lhs, Token& rhs, Token& operator_token) {
    double lhs_val = getDouble(lhs);
    double rhs_val = getDouble(rhs);

    #ifdef DEBUG_RULE_ENGINE
    ESP_LOGD(TAG_RULE_ENGINE, "\n==== Apply Arithemetic Operator ====");
    log_printf("- Operator: %s", operator_token.lexeme.c_str());
    log_printf("\n- LHS Lexeme: %s Value: %f", lhs.lexeme.c_str(), lhs_val);
    log_printf("\n- RHS Lexeme: %s Value: %f", rhs.lexeme.c_str(), rhs_val);
    #endif

    double retval = 0;

    if (operator_token.lexeme == ARITHMETIC_ADD) {
        retval = lhs_val + rhs_val;
    } else if (operator_token.lexeme == ARITHMETIC_SUBTRACT) {
        retval = lhs_val - rhs_val;
    } else if (operator_token.lexeme == ARITHMETIC_MULTIPLY) {
        retval = lhs_val * rhs_val;
    } else if (operator_token.lexeme == ARITHMETIC_DIVIDE) {
        retval = lhs_val / rhs_val;
    } else if (operator_token.lexeme == ARITHMETIC_MODULUS) {
        retval = (double)((int)lhs_val %  (int)rhs_val);
    } else if (operator_token.lexeme == ARITHMETIC_POWER) {
        retval = pow(lhs_val, rhs_val); // lhs ^ rhs
    } else throw std::invalid_argument("Invalid arithmetic operator.");

    #ifdef DEBUG_RULE_ENGINE
    log_printf("\n- Outcome: %f", retval);
    log_printf("\n====================================\r\n\n");
    #endif

    return retval;
}

bool Evaluator::applyComparisonOperator(Token& lhs, Token& rhs, Token& operator_token) {
    if (getVarType(lhs) == UINT64_T || getVarType(rhs) == UINT64_T) return applyComparisonOperatorUint64(lhs, rhs, operator_token);
    double lhs_val = getDouble(lhs);
    double rhs_val = getDouble(rhs);

    #ifdef DEBUG_RULE_ENGINE
    ESP_LOGD(TAG_RULE_ENGINE, "\n==== Apply Comparison Operator ====");
    log_printf("- Operator: %s", operator_token.lexeme.c_str());
    log_printf("\n- LHS Lexeme: %s Value: %f", lhs.lexeme.c_str(), lhs_val);
    log_printf("\n- RHS Lexeme: %s Value: %f", rhs.lexeme.c_str(), rhs_val);
    #endif

    bool retval = false;
    if (operator_token.lexeme == COMPARISON_EQUAL) {
        retval = (lhs_val == rhs_val);
    } else if (operator_token.lexeme == COMPARISON_NOT_EQUAL) {
        retval = (lhs_val != rhs_val);
    } else if (operator_token.lexeme == COMPARISON_GREATER_THAN_OR_EQUAL) {
        retval = (lhs_val >= rhs_val);
    } else if (operator_token.lexeme == COMPARISON_LESSER_THAN_OR_EQUAL) {
        retval = (lhs_val <= rhs_val);
    } else if (operator_token.lexeme == COMPARISON_GREATER_THAN) {
        retval = (lhs_val > rhs_val);
    } else if (operator_token.lexeme == COMPARISON_LESSER_THAN) {
        retval = (lhs_val < rhs_val);
    } else throw std::invalid_argument("Invalid comparison operator.");

    #ifdef DEBUG_RULE_ENGINE
    log_printf("\n- Outcome: %d", (int) retval);
    log_printf("\n====================================\r\n\n");
    #endif

    return retval;
}

bool Evaluator::applyComparisonOperatorUint64(Token& lhs, Token& rhs, Token& operator_token) {
    uint64_t lhs_val = getUint64(lhs);
    uint64_t rhs_val = getUint64(rhs);

    #ifdef DEBUG_RULE_ENGINE
    ESP_LOGD(TAG_RULE_ENGINE, "\n==== Apply Comparison Operator uint64_t ====");
    log_printf("- Operator: %s", operator_token.lexeme.c_str());
    log_printf("\n- LHS Lexeme: %s Value: %u", lhs.lexeme.c_str(), lhs_val);
    log_printf("\n- RHS Lexeme: %s Value: %u", rhs.lexeme.c_str(), rhs_val);
    #endif

    bool retval = false;
    if (operator_token.lexeme == COMPARISON_EQUAL) {
        retval = (lhs_val == rhs_val);
    } else if (operator_token.lexeme == COMPARISON_NOT_EQUAL) {
        retval = (lhs_val != rhs_val);
    } else if (operator_token.lexeme == COMPARISON_GREATER_THAN_OR_EQUAL) {
        retval = (lhs_val >= rhs_val);
    } else if (operator_token.lexeme == COMPARISON_LESSER_THAN_OR_EQUAL) {
        retval = (lhs_val <= rhs_val);
    } else if (operator_token.lexeme == COMPARISON_GREATER_THAN) {
        retval = (lhs_val > rhs_val);
    } else if (operator_token.lexeme == COMPARISON_LESSER_THAN) {
        retval = (lhs_val < rhs_val);
    } else throw std::invalid_argument("Invalid comparison operator.");

    #ifdef DEBUG_RULE_ENGINE
    log_printf("\n- Outcome: %d", (int) retval);
    log_printf("\n============================================\r\n\n");
    #endif

    return retval;
}

/**
 * @brief Handles the comparison between two arrays or an array and a string literal/variable.
*/
bool Evaluator::applyArrayComparison(Token& lhs, Token& rhs, Token& operator_token) {
    ArraySeparator separator;
    ps_vector<ps_string> search_array; 
    Token arr_name;

    VarType lhs_type = getVarType(lhs);
    VarType rhs_type = getVarType(rhs);

    if (lhs.type == ARRAY) {
        search_array = separator.separate(lhs);
        arr_name = rhs;
    } else if (lhs.type == STRING_LITERAL) {
        search_array.push_back(lhs.lexeme);
        arr_name = rhs;
    } else if (lhs_type == PS_STRING) {
        search_array.push_back(getString(lhs));
        arr_name = rhs;
    } else if (rhs.type == ARRAY) {
        search_array = separator.separate(rhs);
        arr_name = lhs;
    } else if (rhs.type == STRING_LITERAL) {
        search_array.push_back(rhs.lexeme);
        arr_name = lhs;
    } else if (rhs_type == PS_STRING) {
        search_array.push_back(getString(rhs));
        arr_name = lhs;
    }

    if (search_array.size() == 0) throw std::invalid_argument("Invalid token type for array comparison.");



    #ifdef DEBUG_RULE_ENGINE
    ESP_LOGD(TAG_RULE_ENGINE, "\n==== Apply Comparison Operator Array ====");
    ps_string array_str;
    array_str += "[";
    for (size_t i = 0; i < search_array.size(); i++) {
        array_str += search_array.at(i);
        array_str += ", ";
    };
    array_str += "]";
    
    log_printf("- Operator: %s", operator_token.lexeme.c_str());
    log_printf("\n- Array Name: %s", arr_name.lexeme.c_str());
    log_printf("\n- Search Array: %s", array_str.c_str());
    #endif

    bool retval;

    if (operator_token.lexeme == ARRAY_TAG_EQUALITY_COMPARISON) {
        if (arr_name.lexeme == UNIT_TAG_LIST) {
            retval = global_class.tagEqualityComparison(search_array);
        } else if (arr_name.lexeme == MODULE_TAG_LIST) {
            retval = module_class.tagEqualityComparison(search_array);
        } else throw std::invalid_argument("Unknown array name."); 
    } else if (operator_token.lexeme == ARRAY_TAG_SUBSET_COMPARISON) {
        if (arr_name.lexeme == UNIT_TAG_LIST) {
            retval = global_class.tagSubsetComparison(search_array);
        } else if (arr_name.lexeme == MODULE_TAG_LIST) {
            retval = module_class.tagSubsetComparison(search_array);
        } else throw std::invalid_argument("Unknown array name."); 
    } else throw std::invalid_argument("Unknown array comparison.");

    #ifdef DEBUG_RULE_ENGINE
    log_printf("\n- Outcome: %d", (int) retval);
    log_printf("\n============================================\r\n\n");
    #endif
    return retval;
}

/**
 * @brief Handles the comparison between two string literals or variables of string literal type.
*/
bool Evaluator::applyStringComparison(Token& lhs, Token& rhs, Token& operator_token) {
    ps_string lhs_str = getString(lhs);
    ps_string rhs_str = getString(rhs);

    #ifdef DEBUG_RULE_ENGINE
    ESP_LOGD(TAG_RULE_ENGINE, "\n==== Apply Comparison Operator String ====");
    log_printf("- Operator: %s", operator_token.lexeme.c_str());
    log_printf("\n- LHS Lexeme: %s Value: %s", lhs.lexeme.c_str(), lhs_str.c_str());
    log_printf("\n- RHS Lexeme: %s Value: %s", rhs.lexeme.c_str(), rhs_str.c_str());
    #endif

    bool retval;

    if (operator_token.lexeme == COMPARISON_EQUAL) {
        retval = (lhs_str == rhs_str);
    } else if (operator_token.lexeme == COMPARISON_NOT_EQUAL) {
        retval = (lhs_str != rhs_str);
    } else throw std::invalid_argument("No matching string comparison found.");

    #ifdef DEBUG_RULE_ENGINE
    log_printf("\n- Outcome: %d", (int) retval);
    log_printf("\n============================================\r\n\n");
    #endif

    return retval;
}

const double Evaluator::getDouble(Token& token) {
    double retval = false;

    switch(token.type) {
        case IDENTIFIER:
            //ESP_LOGD(TAG_RULE_ENGINE, "Getting double value of IDENTIFIER token with lexeme: \'%s\'", token.lexeme.c_str());
            switch(getVarType(token)) {
                case DOUBLE:
                    retrieveVar(token.lexeme, retval);
                    break;
                case INT:
                    int val;
                    retrieveVar(token.lexeme, val);
                    retval = (double) val;
                    break;
                case BOOL:
                    bool bool_val;
                    retrieveVar(token.lexeme, bool_val);
                    retval = (double) bool_val;
                    break;
                case UINT64_T:
                    uint64_t uint64_val;
                    retrieveVar(token.lexeme, uint64_val);
                    retval = (double) val;
                    break;
                default:
                    throw std::invalid_argument("Cannot get double of this token type.");
                break;
            }
        break;
        case NUMERIC_LITERAL:
        //ESP_LOGD(TAG_RULE_ENGINE, "Getting double value of NUMERIC_LITERAL token with lexeme: \'%s\'", token.lexeme.c_str());
        retval = toDouble(token.lexeme);
        break;
    }

    return retval;
}

const bool Evaluator::getBool(Token& token) {
    bool retval = false;

    switch(token.type) {
        case IDENTIFIER:
        //ESP_LOGD(TAG_RULE_ENGINE, "Getting bool value of IDENTIFIER token with lexeme: \'%s\'", token.lexeme.c_str());
            switch(getVarType(token)) {
                case DOUBLE:
                    double val;
                    retrieveVar(token.lexeme, val);
                    retval = (bool) val;
                    break;
                case INT:
                    int int_val;
                    retrieveVar(token.lexeme, val);
                    retval = (bool) int_val;
                    break;
                case BOOL:
                    retrieveVar(token.lexeme, retval);
                    break;
                case UINT64_T:
                    uint64_t uint64_val;
                    retrieveVar(token.lexeme, val);
                    retval = (bool) uint64_val;
                    break;
                default:
                    throw std::invalid_argument("Cannot get bool of this token type.");
                break;
            }
        break;
        case NUMERIC_LITERAL:
       // ESP_LOGD(TAG_RULE_ENGINE, "Getting bool value of NUMERIC_LITERAL token with lexeme: \'%s\'", token.lexeme.c_str());
        retval = (bool) toDouble(token.lexeme);
        break;
    }

    return retval;
}

const int Evaluator::getInt(Token& token) {
    int retval = false;

    switch(token.type) {
        case IDENTIFIER:
            switch(getVarType(token)) {
                case DOUBLE:
                    double val;
                    retrieveVar(token.lexeme, val);
                    retval = (int) val;
                    break;
                case INT:
                    retrieveVar(token.lexeme, retval);
                    break;
                case BOOL:
                    bool bool_val;
                    retrieveVar(token.lexeme, val);
                    retval = (int) bool_val;
                    break;
                case UINT64_T:
                    uint64_t uint64_val;
                    retrieveVar(token.lexeme, val);
                    retval = (int) uint64_val;
                    break;
                default:
                    throw std::invalid_argument("Cannot get int of this token type.");
                break;
            }
        break;
        case NUMERIC_LITERAL:
        retval = (int) toDouble(token.lexeme);
        break;
    }

    return retval;
}

const uint64_t Evaluator::getUint64(Token& token) {
    uint64_t retval;

    switch(token.type) {
        case IDENTIFIER:
            switch(getVarType(token)) {
                case DOUBLE:
                    double val;
                    retrieveVar(token.lexeme, val);
                    retval = (uint64_t) val;
                    break;
                case INT:
                    int int_val;
                    retrieveVar(token.lexeme, val);
                    retval = (uint64_t) int_val;
                    break;
                case BOOL:
                    bool bool_val;
                    retrieveVar(token.lexeme, val);
                    retval = (uint64_t) bool_val;
                    break;
                case UINT64_T:
                    retrieveVar(token.lexeme, retval);
                    break;
                default:
                    throw std::invalid_argument("Cannot get bool of this token type.");
                break;
            }
        break;
        case NUMERIC_LITERAL:
        std::string val_str;
        val_str <<= token.lexeme;

        std::istringstream iss(val_str);

        iss >> retval;
        break;
    }

    return retval;
}

const ps_string Evaluator::getString(Token& token) {
    ps_string retval;

    if (token.type == STRING_LITERAL) return token.lexeme;
    if (token.type != IDENTIFIER) throw std::invalid_argument("Cannot get ps_string of this token type.");

    retrieveVar(token.lexeme, retval);

    return retval;
}


double Evaluator::toDouble(ps_string& str) {
    std::string string;
    string <<= str;

    return std::atof(string.c_str());
}

const VarType&  Evaluator::getVarType(const Token& search_token) {
    std::string string;
    string <<= search_token.lexeme;

    auto result = vartype_lookup.find(string);

    if (result == vartype_lookup.end()) {
        return vartype_lookup.find("INVALID_VAR") -> second;
    }

    return result -> second;
}

void Evaluator::retrieveVar(const ps_string& var, double& val) {
    std::string var_name;
    var_name <<= var;

    static const std::unordered_map<std::string, std::function<double()>> varMap = {
        {TOTAL_ACTIVE_POWER, [this](){ return global_class.totalActivePower(); }},
        {TOTAL_REACTIVE_POWER, [this](){ return global_class.totalReactivePower(); }},
        {TOTAL_APPARENT_POWER, [this](){ return global_class.totalApparentPower(); }},
        {ACTIVE_POWER, [this](){ return module_class.latestReading().active_power; }},
        {REACTIVE_POWER, [this](){ return module_class.latestReading().reactive_power; }},
        {APPARENT_POWER, [this](){ return module_class.latestReading().apparent_power; }},
        {VOLTAGE, [this](){ return module_class.latestReading().voltage; }},
        {FREQUENCY, [this](){ return module_class.latestReading().frequency; }},
        {POWER_FACTOR, [this](){ return module_class.latestReading().power_factor; }}
    };

    auto it = varMap.find(var_name);
    if (it != varMap.end()) {
        val = it->second();
    } else {
        throw std::invalid_argument("Invalid variable of type \'double\' requested.");
    }
}

void Evaluator::retrieveVar(const ps_string& var, bool& val) {
    std::string var_name;
    var_name <<= var;

    static const std::unordered_map<std::string, std::function<bool()>> varMap = {
        {SWITCH_STATUS, [this](){ return module_class.status(); }},
        {POWER_STATUS, [this](){ return global_class.powerStatus(); }}
    };

    auto it = varMap.find(var_name);
    if (it != varMap.end()) {
        val = it->second();
    } else {
        throw std::invalid_argument("Invalid variable of type \'bool\' requested.");
    }

    return;
}

void Evaluator::retrieveVar(const ps_string& var, int& val) {
    std::string var_name;
    var_name <<= var;

    static const std::unordered_map<std::string, std::function<int()>> varMap = {
        {CIRCUIT_PRIORITY, [this](){ return module_class.priority(); }},
        {MODULE_COUNT, [this](){ return global_class.moduleCount(); }}
    };

    auto it = varMap.find(var_name);
    if (it != varMap.end()) {
        val = it->second();
    } else throw std::invalid_argument("Invalid variable of type \'int\' requested.");

    return;
}

void Evaluator::retrieveVar(const ps_string& var, uint64_t& val) {
    std::string var_name;
    var_name <<= var;

    static const std::unordered_map<std::string, std::function<uint64_t()>> varMap = {
        {CURRENT_TIME, [this](){ return (uint64_t) time(nullptr); }},
        {SWITCH_TIME, [this](){ return global_class.moduleCount(); }}
    };

    auto it = varMap.find(var_name);
    if (it != varMap.end()) {
        val = it->second();
    } else throw std::invalid_argument("Invalid variable of type \'uint64_t\' requested.");

    return;
}

void Evaluator::retrieveVar(const ps_string& var, ps_string& val) {
    if (var == MODULE_ID) {
        val = module_class.id();
    } else if (var == UNIT_ID) {
        val = global_class.id();
    } else throw std::invalid_argument("Invalid variable of type \'ps_string\' requested.");

    return;
}


