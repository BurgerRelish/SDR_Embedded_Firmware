#include "Evaluator.h"
#include <math.h>
#include <time.h>
#include <iostream>

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
    }   
}

ps_queue<Token> Evaluator::evaluate() {
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
                token_stack.push(token_list.front()); // Push all literals and identifiers to the stack.
                break;
            case ARITHMETIC_OPERATOR: // + - / ^ %
            case BOOLEAN_OPERATOR: // && || !
            case COMPARISON_OPERATOR: // == != <= >= < >
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
    tokens.pop();
    Token lhs = tokens.top();
    tokens.pop();

    Token result;
    result.type = NUMERIC_LITERAL;
    std::string val_str;

    // Handle arrays separately
    if (lhs.type == ARRAY || rhs.type == ARRAY || lhs.type == STRING_LITERAL || rhs.type == STRING_LITERAL) {
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

    if (operator_token.lexeme == BOOLEAN_AND) {
        return ((bool) lhs_val && (bool) rhs_val);
    } else if (operator_token.lexeme == BOOLEAN_OR) {
        return ((bool) lhs_val && (bool) rhs_val);
    } else if (operator_token.lexeme == BOOLEAN_NOT) {
        return (!(bool) lhs_val);
    } else throw std::invalid_argument("Invalid Boolean Operator");
}

double Evaluator::applyArithmeticOperator(Token& lhs, Token& rhs, Token& operator_token) {
    double lhs_val = getDouble(lhs);
    double rhs_val = getDouble(rhs);

    if (operator_token.lexeme == ARITHMETIC_ADD) {
        return lhs_val + rhs_val;
    } else if (operator_token.lexeme == ARITHMETIC_SUBTRACT) {
        return lhs_val - rhs_val;
    } else if (operator_token.lexeme == ARITHMETIC_MULTIPLY) {
        return lhs_val * rhs_val;
    } else if (operator_token.lexeme == ARITHMETIC_DIVIDE) {
        return lhs_val / rhs_val;
    } else if (operator_token.lexeme == ARITHMETIC_MODULUS) {
        return (double)((int)lhs_val %  (int)rhs_val);
    } else if (operator_token.lexeme == ARITHMETIC_POWER) {
        return pow(lhs_val, rhs_val); // lhs ^ rhs
    } else throw std::invalid_argument("Invalid arithmetic operator.");
}

bool Evaluator::applyComparisonOperator(Token& lhs, Token& rhs, Token& operator_token) {
    double lhs_val = getDouble(lhs);
    double rhs_val = getDouble(rhs);

    if (operator_token.lexeme == COMPARISON_EQUAL) {
        return (lhs_val == rhs_val);
    } else if (operator_token.lexeme == COMPARISON_NOT_EQUAL) {
        return (lhs_val != rhs_val);
    } else if (operator_token.lexeme == COMPARISON_GREATER_THAN_OR_EQUAL) {
        return (lhs_val >= rhs_val);
    } else if (operator_token.lexeme == COMPARISON_LESSER_THAN_OR_EQUAL) {
        return (lhs_val <= rhs_val);
    } else if (operator_token.lexeme == COMPARISON_GREATER_THAN) {
        return (lhs_val > rhs_val);
    } else if (operator_token.lexeme == COMPARISON_LESSER_THAN) {
        return (lhs_val < rhs_val);
    } else throw std::invalid_argument("Invalid comparison operator.");
}

bool Evaluator::applyArrayComparison(Token& lhs, Token& rhs, Token& operator_token) {
    ArraySeparator separator;
    ps_vector<ps_string> array;

    Token arr_name;

    if (lhs.type == ARRAY) {
        array = separator.separate(lhs);
        arr_name = rhs;
    } else if (rhs.type == ARRAY) {
        array = separator.separate(rhs);
        arr_name = lhs;
    } else if (lhs.type == STRING_LITERAL) {
        arr_name = rhs;
        array.push_back(lhs.lexeme);
    } else if (rhs.type == STRING_LITERAL) {
        arr_name = lhs;
        array.push_back(rhs.lexeme);
    } else throw std::invalid_argument("Invalid token type for array comparison.");

    if (operator_token.lexeme == ARRAY_TAG_EQUALITY_COMPARISON) {
        if (arr_name.lexeme == UNIT_TAG_LIST) {
            return global_class.tagEqualityComparison(array);
        } else if (arr_name.lexeme == MODULE_TAG_LIST) {
            return module_class.tagEqualityComparison(array);
        }
    } else if (operator_token.lexeme == ARRAY_TAG_SUBSET_COMPARISON) {
        if (arr_name.lexeme == UNIT_TAG_LIST) {
            return global_class.tagSubsetComparison(array);
        } else if (arr_name.lexeme == MODULE_TAG_LIST) {
            return module_class.tagSubsetComparison(array);
        } 
    } else throw std::invalid_argument("Unknown array comparison.");

    throw std::invalid_argument("Unknown array name."); 
}

const double Evaluator::getDouble(Token& token) {
    double retval = false;

    switch(token.type) {
        case IDENTIFIER:
            switch(getVarType(token.lexeme)) {
                case DOUBLE:
                    retrieveVar(token.lexeme, retval);
                    break;
                case INT:
                    int val;
                    retrieveVar(token.lexeme, val);
                    retval = (double) val;
                    break;
                case BOOL:
                    bool val;
                    retrieveVar(token.lexeme, val);
                    retval = (double) val;
                    break;
                case UINT64_T:
                    uint64_t val;
                    retrieveVar(token.lexeme, val);
                    retval = (double) val;
                    break;
                default:
                    throw std::invalid_argument("Cannot get double of this token type.");
                break;
            }
        break;
        case NUMERIC_LITERAL:
        retval = (bool) toDouble(token.lexeme);
        break;
    }

    return retval;
}

const bool Evaluator::getBool(Token& token) {
    bool retval = false;

    switch(token.type) {
        case IDENTIFIER:
            switch(getVarType(token.lexeme)) {
                case DOUBLE:
                    double val;
                    retrieveVar(token.lexeme, val);
                    retval = (bool) val;
                    break;
                case INT:
                    int val;
                    retrieveVar(token.lexeme, val);
                    retval = (bool) val;
                    break;
                case BOOL:
                    retrieveVar(token.lexeme, retval);
                    break;
                case UINT64_T:
                    uint64_t val;
                    retrieveVar(token.lexeme, val);
                    retval = (bool) val;
                    break;
                default:
                    throw std::invalid_argument("Cannot get bool of this token type.");
                break;
            }
        break;
        case NUMERIC_LITERAL:
        retval = (bool) toDouble(token.lexeme);
        break;
    }

    return retval;
}

const int Evaluator::getInt(Token& token) {
    int retval = false;

    switch(token.type) {
        case IDENTIFIER:
            switch(getVarType(token.lexeme)) {
                case DOUBLE:
                    double val;
                    retrieveVar(token.lexeme, val);
                    retval = (int) val;
                    break;
                case INT:
                    retrieveVar(token.lexeme, retval);
                    break;
                case BOOL:
                    bool val;
                    retrieveVar(token.lexeme, val);
                    retval = (int) val;
                    break;
                case UINT64_T:
                    uint64_t val;
                    retrieveVar(token.lexeme, val);
                    retval = (int) val;
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
            switch(getVarType(token.lexeme)) {
                case DOUBLE:
                    double val;
                    retrieveVar(token.lexeme, val);
                    retval = (uint64_t) val;
                    break;
                case INT:
                    int val;
                    retrieveVar(token.lexeme, val);
                    retval = (uint64_t) val;
                    break;
                case BOOL:
                    bool val;
                    retrieveVar(token.lexeme, val);
                    retval = (uint64_t) val;
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
        retval = (bool) toDouble(token.lexeme);
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

const VarType&  Evaluator::getVarType(ps_string& search_str) {
    std::string string;
    string <<= search_str;

    auto result = vartype_lookup.find(string);

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


