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

void Evaluator::generateRules(const ps_vector<Rule>& rule_input) {
    Rule current_rule;
    LexedRule new_rule;
    ShuntingYard rpn_algorithm;
    ps_vector<Rule> _rule_input = rule_input;

    while(!_rule_input.empty()) {
        current_rule = _rule_input.at(0);
        new_rule.priority = current_rule.priority;

        // Lex each rule and convert it to postfix notation, ready for evaluation. 
        ps_queue<Token> tokens = lexExpression(current_rule.expression);
        new_rule.expression = rpn_algorithm.apply(tokens);

        new_rule.commands = lexExpression(current_rule.command);

        rules.push_back(new_rule);
        _rule_input.erase(_rule_input.begin());
    }   
}

Command Evaluator::evaluate() {
    if (esp_timer_get_time() < delay_end_time) {
        Command ret;
        ret.type = origin;
        ret.priority = 99999;
        ret.command = ps_queue<Token>();
        ret.origin = shared_from_this();
        return ret;
    }

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

    Command ret;
    ret.command = commands;
    ret.type = origin;
    ret.origin = shared_from_this();
    ret.priority = highest_priority;

    return ret;
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
            retval = unit->tagEqualityComparison(search_array);
        } else if (arr_name.lexeme == MODULE_TAG_LIST) {
            retval = module->tagEqualityComparison(search_array);
        } else throw std::invalid_argument("Unknown array name."); 
    } else if (operator_token.lexeme == ARRAY_TAG_SUBSET_COMPARISON) {
        if (arr_name.lexeme == UNIT_TAG_LIST) {
            retval = unit->tagSubsetComparison(search_array);
        } else if (arr_name.lexeme == MODULE_TAG_LIST) {
            retval = module->tagSubsetComparison(search_array);
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