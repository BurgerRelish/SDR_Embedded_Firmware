#include "Expression.h"

namespace re {

bool Expression::evaluate() {
    #ifdef DEBUG_RULE_ENGINE
    uint64_t start_time = esp_timer_get_time();
    #endif

    bool ret = evaluateRPN();

    #ifdef DEBUG_RULE_ENGINE
    uint64_t tot_time = (esp_timer_get_time() - start_time);
    
    ESP_LOGD("Expr", "\n==== Rule Evaluation Completed ====");
    log_printf("- Processing Time: %uus\n", tot_time);
    log_printf("- Outcome: %d\n", ret);
    #endif

    return ret;
}

bool Expression::evaluateRPN() {
    ps::queue<Token> token_list = _expression;
    ps::stack<Token> token_stack;
    
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

void Expression::evaluateOperator(ps::stack<Token>& tokens, Token& operator_token) {
    Token rhs = tokens.top();
    VariableType rhs_type = variables->type(rhs.lexeme);
    tokens.pop();

    Token lhs = tokens.top();
    VariableType lhs_type = variables->type(lhs.lexeme);
    tokens.pop();

    Token result;
    result.type = NUMERIC_LITERAL;
    std::string val_str;

    // Handle arrays and strings separately
    if ((lhs_type == VAR_STRING && rhs.type == STRING_LITERAL) || (rhs_type == VAR_STRING && lhs.type == STRING_LITERAL) || (lhs_type == VAR_STRING && rhs_type == VAR_STRING)) {
        val_str = std::to_string((double) applyStringComparison(lhs, rhs, operator_token));
        result.lexeme <<= val_str;
        tokens.push(result);
        return;
    } else if (lhs_type == VAR_ARRAY || rhs_type == VAR_ARRAY) {
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

bool Expression::applyBooleanOperator(Token& lhs, Token& rhs, Token& operator_token) {
    bool lhs_val = variables->get<bool>(lhs.lexeme);
    bool rhs_val = variables->get<bool>(rhs.lexeme);

    #ifdef DEBUG_RULE_ENGINE
    ESP_LOGD("bool", "\n==== Apply Operator ====");
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
    log_printf("\n============================\r\n\n");
    #endif

    return retval;
}

double Expression::applyArithmeticOperator(Token& lhs, Token& rhs, Token& operator_token) {
    double lhs_val = variables->get<double>(lhs.lexeme);
    double rhs_val = variables->get<double>(rhs.lexeme);

    #ifdef DEBUG_RULE_ENGINE
    ESP_LOGD("Arithmetic", "\n==== Apply Operator ====");
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
    log_printf("\n=========================\r\n\n");
    #endif

    return retval;
}

bool Expression::applyComparisonOperator(Token& lhs, Token& rhs, Token& operator_token) {
    if (variables->type(lhs.lexeme) == VAR_UINT64_T || variables->type(rhs.lexeme) == VAR_UINT64_T) return applyComparisonOperatorUint64(lhs, rhs, operator_token);
    double lhs_val = variables->get<double>(lhs.lexeme);
    double rhs_val = variables->get<double>(rhs.lexeme);

    #ifdef DEBUG_RULE_ENGINE
    ESP_LOGD("Comparison", "\n==== Apply Operator ====");
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
    log_printf("\n==========================\r\n\n");
    #endif

    return retval;
}

bool Expression::applyComparisonOperatorUint64(Token& lhs, Token& rhs, Token& operator_token) {
    uint64_t lhs_val = variables->get<uint64_t>(lhs.lexeme);
    uint64_t rhs_val = variables->get<uint64_t>(rhs.lexeme);

    #ifdef DEBUG_RULE_ENGINE
    ESP_LOGD("uint64_t", "\n==== Apply Comparison Operator ====");
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
    log_printf("\n===================================\r\n\n");
    #endif

    return retval;
}

/**
 * @brief Handles the comparison between two arrays or an array and a string literal/variable.
*/
bool Expression::applyArrayComparison(Token& lhs, Token& rhs, Token& operator_token) {
    ArraySeparator separator;
    ps::vector<ps::string> search_array; 
    Token arr_name;

    VariableType lhs_type = variables->type(lhs.lexeme);
    VariableType rhs_type = variables->type(rhs.lexeme);

    if (lhs.type == ARRAY) {
        search_array = separator.separate(lhs);
        arr_name = rhs;
    } else if (lhs.type == STRING_LITERAL) {
        search_array.push_back(lhs.lexeme);
        arr_name = rhs;
    } else if (lhs_type == VAR_STRING) {
        search_array.push_back(variables->get<ps::string>(lhs.lexeme));
        arr_name = rhs;
    } else if (rhs.type == ARRAY) {
        search_array = separator.separate(rhs);
        arr_name = lhs;
    } else if (rhs.type == STRING_LITERAL) {
        search_array.push_back(rhs.lexeme);
        arr_name = lhs;
    } else if (rhs_type == VAR_STRING) {
        search_array.push_back(variables->get<ps::string>(rhs.lexeme));
        arr_name = lhs;
    }

    if (search_array.size() == 0) throw std::invalid_argument("Invalid token type for array comparison.");

    #ifdef DEBUG_RULE_ENGINE
    ESP_LOGD("Array", "\n==== Apply Comparison Operator ====");
    ps::string array_str;
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
            retval = variables->get_direct<std::shared_ptr<SDRUnit>>("UNIT")->tagEqualityComparison(search_array);
        } else if (arr_name.lexeme == MODULE_TAG_LIST) {
            retval = variables->get_direct<std::shared_ptr<Module>>("MODULE")->tagEqualityComparison(search_array);
        } else throw std::invalid_argument("Unknown array name."); 
    } else if (operator_token.lexeme == ARRAY_TAG_SUBSET_COMPARISON) {
        if (arr_name.lexeme == UNIT_TAG_LIST) {
            retval = variables->get_direct<std::shared_ptr<SDRUnit>>("UNIT")->tagSubsetComparison(search_array);
        } else if (arr_name.lexeme == MODULE_TAG_LIST) {
            retval = variables->get_direct<std::shared_ptr<Module>>("MODULE")->tagSubsetComparison(search_array);
        } else throw std::invalid_argument("Unknown array name."); 
    } else throw std::invalid_argument("Unknown array comparison.");

    #ifdef DEBUG_RULE_ENGINE
    log_printf("\n- Outcome: %d", (int) retval);
    log_printf("\n======================================\r\n\n");
    #endif
    return retval;
}

/**
 * @brief Handles the comparison between two string literals or variables of string literal type.
*/
bool Expression::applyStringComparison(Token& lhs, Token& rhs, Token& operator_token) {
    ps::string lhs_str = variables->get<ps::string>(lhs.lexeme);
    ps::string rhs_str = variables->get<ps::string>(rhs.lexeme);

    #ifdef DEBUG_RULE_ENGINE
    ESP_LOGD("", "\n==== Apply Comparison Operator String ====");
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
}