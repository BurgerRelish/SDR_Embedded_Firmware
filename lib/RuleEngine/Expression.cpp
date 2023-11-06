#include "Expression.h"

namespace re {

bool Expression::evaluate() {
    #ifdef DEBUG_RULE_ENGINE
    uint64_t start_time = esp_timer_get_time();
    #endif

    bool ret = evaluateRPN();

    #ifdef DEBUG_RULE_ENGINE
    uint64_t end_time = esp_timer_get_time();
    
    ESP_LOGD("Expr", "\n==== Rule Evaluation Completed ====");
    log_printf("- Processing Time: %uus\n", end_time - start_time);
    log_printf("- Outcome: %d\n", ret);
    #endif

    return ret;
}

double Expression::result() {
    return evaluateRPN();
}

double Expression::evaluateRPN() {
    ps::queue<Token> token_list;
    for (auto token : _expression) {
        token_list.push(token);
    }
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

    return std::atof(token_stack.top().lexeme.c_str());
}

void Expression::evaluateOperator(ps::stack<Token>& tokens, Token& operator_token) {
    Token rhs = tokens.top();
    VariableType rhs_type = variables->get_type(rhs.lexeme);
    tokens.pop();

    Token lhs = tokens.top();
    VariableType lhs_type = variables->get_type(lhs.lexeme);
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
        if (operator_token.lexeme == "&&" || operator_token.lexeme == "||") {
            result = applyArrayOperator(lhs, rhs, operator_token);
        } else {
            val_str = std::to_string((double) applyArrayComparison(lhs, rhs, operator_token));
            result.lexeme <<= val_str; 
        }

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
    bool lhs_val = variables->get_var<bool>(lhs.lexeme);
    bool rhs_val = variables->get_var<bool>(rhs.lexeme);

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
        retval = ((bool) lhs_val || (bool) rhs_val);
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
    double lhs_val = variables->get_var<double>(lhs.lexeme);
    double rhs_val = variables->get_var<double>(rhs.lexeme);

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
    if (variables->get_type(lhs.lexeme) == VAR_UINT64_T || variables->get_type(rhs.lexeme) == VAR_UINT64_T) return applyComparisonOperatorUint64(lhs, rhs, operator_token);
    
    double lhs_val = variables->get_var<double>(lhs.lexeme);
    double rhs_val = variables->get_var<double>(rhs.lexeme);

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
    uint64_t lhs_val = variables->get_var<uint64_t>(lhs.lexeme);
    uint64_t rhs_val = variables->get_var<uint64_t>(rhs.lexeme);

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
    ps::vector<ps::string> lhs_array;
    ps::vector<ps::string> rhs_array; 

    Token arr_name;

    VariableType lhs_type = variables->get_type(lhs.lexeme);
    VariableType rhs_type = variables->get_type(rhs.lexeme);
    /* If the array is of a string type, */
    if (lhs_type == VAR_STRING || lhs.type == STRING_LITERAL) {
        lhs_array.push_back(variables->get_var<ps::string>(lhs.lexeme));
    } else if (lhs.type == ARRAY) {
        auto it = array_lookup.find(rhs.lexeme);
        if (it != array_lookup.end()) { // Array already exists.
            lhs_array = it -> second;
        } else { // Else parse the array and store it for later.
            lhs_array = separateArray(lhs);
            array_lookup.insert(std::make_pair(rhs.lexeme, lhs_array));
        }
    } else if (lhs_type == VAR_ARRAY) {
        lhs_array = variables->get_var<ps::vector<ps::string>>(lhs.lexeme);
    }

    if (rhs_type == VAR_STRING || rhs.type == STRING_LITERAL) {
        rhs_array.push_back(variables->get_var<ps::string>(rhs.lexeme));
    } else if (rhs.type == ARRAY) {
        auto it = array_lookup.find(lhs.lexeme);
        if (it != array_lookup.end()) { // Array already exists.
            rhs_array = it -> second;
        } else { // Else parse the array and store it for later.
            rhs_array = separateArray(rhs);
            array_lookup.insert(std::make_pair(lhs.lexeme, rhs_array));
        }
    } else if (rhs_type == VAR_ARRAY) {
        rhs_array = variables->get_var<ps::vector<ps::string>>(rhs.lexeme);
    }

    #ifdef DEBUG_RULE_ENGINE
    ESP_LOGD("Array", "\n==== Apply Comparison Operator ====");
    ps::string array_str;
    log_printf("- Operator: %s", operator_token.lexeme.c_str());

    array_str += "[";
    for (size_t i = 0; i < lhs_array.size(); i++) {
        array_str += lhs_array.at(i);
        array_str += ", ";
    };
    array_str += "]";
    log_printf("\n- lhs Array: %s", array_str.c_str());

    array_str.clear();
    array_str += "[";
    for (size_t i = 0; i < rhs_array.size(); i++) {
        array_str += rhs_array.at(i);
        array_str += ", ";
    };
    array_str += "]";
    log_printf("\n- rhs Array: %s", array_str.c_str());
    
    #endif

    bool retval;

    if (operator_token.lexeme == ARRAY_TAG_EQUALITY_COMPARISON) {
        retval = arrayEqualityComparison(lhs_array, rhs_array);
    } else if (operator_token.lexeme == ARRAY_TAG_INEQUALITY_COMPARISON) {
        retval = !arrayEqualityComparison(lhs_array, rhs_array);
    } else if (operator_token.lexeme == ARRAY_TAG_SUBSET_COMPARISON) {
        retval = arraySubsetComparison(lhs_array, rhs_array);
    } else throw std::invalid_argument("Unknown array comparison.");

    #ifdef DEBUG_RULE_ENGINE
    log_printf("\n- Outcome: %d", (int) retval);
    log_printf("\n======================================\r\n\n");
    #endif
    return retval;
}

Token Expression::applyArrayOperator(Token& lhs, Token& rhs, Token& operator_token) {
    ps::vector<ps::string> lhs_array;
    ps::vector<ps::string> rhs_array; 

    Token arr_name;

    VariableType lhs_type = variables->get_type(lhs.lexeme);
    VariableType rhs_type = variables->get_type(rhs.lexeme);
    /* If the array is of a string type, */
    if (lhs_type == VAR_STRING || lhs.type == STRING_LITERAL) {
        lhs_array.push_back(variables->get_var<ps::string>(lhs.lexeme));
    } else if (lhs.type == ARRAY) {
        auto it = array_lookup.find(rhs.lexeme);
        if (it != array_lookup.end()) { // Array already exists.
            lhs_array = it -> second;
        } else { // Else parse the array and store it for later.
            lhs_array = separateArray(lhs);
            array_lookup.insert(std::make_pair(rhs.lexeme, lhs_array));
        }
    } else if (lhs_type == VAR_ARRAY) {
        lhs_array = variables->get_var<ps::vector<ps::string>>(lhs.lexeme);
    }

    if (rhs_type == VAR_STRING || rhs.type == STRING_LITERAL) {
        rhs_array.push_back(variables->get_var<ps::string>(rhs.lexeme));
    } else if (rhs.type == ARRAY) {
        auto it = array_lookup.find(lhs.lexeme);
        if (it != array_lookup.end()) { // Array already exists.
            rhs_array = it -> second;
        } else { // Else parse the array and store it for later.
            rhs_array = separateArray(rhs);
            array_lookup.insert(std::make_pair(lhs.lexeme, rhs_array));
        }
    } else if (rhs_type == VAR_ARRAY) {
        rhs_array = variables->get_var<ps::vector<ps::string>>(rhs.lexeme);
    }

    ps::vector<ps::string> retval;
    if (operator_token.lexeme == "||") { // Get unique elements
        // Combine the two arrays, adding the elements if they are not already in the array.
        for (auto& item : lhs_array) {
            if (std::find(retval.begin(), retval.end(), item) == retval.end()) retval.push_back(item);
        }

        for (auto& item : rhs_array) {
            if (std::find(retval.begin(), retval.end(), item) == retval.end()) retval.push_back(item);
        }

    } else if (operator_token.lexeme == "&&") { // Get common elements
        for (auto& item : lhs_array) {
            if (std::find(rhs_array.begin(), rhs_array.end(), item) != rhs_array.end()) retval.push_back(item);
        }
    }

    // Convert the return array back into a string.
    return combineArray(retval);
}

/**
 * @brief Handles the comparison between two string literals or variables of string literal type.
*/
bool Expression::applyStringComparison(Token& lhs, Token& rhs, Token& operator_token) {
    ps::string lhs_str = variables->get_var<ps::string>(lhs.lexeme);
    ps::string rhs_str = variables->get_var<ps::string>(rhs.lexeme);

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



/**
 * @brief Check whether a minimum number of elements match.
 * @param lhs_array The list of elements to compare.
 * @param lhs_array The list of elements to compare.
 * @param n The minimum number of matching elements before a true is returned.
 * @returns True if matching elements >= n, else false.
*/
const bool Expression::arrayMinQuantifierSearch(const ps::vector<ps::string>& lhs_array, const ps::vector<ps::string>& rhs_array, const size_t n) const {
    if ((lhs_array.size() < n) || (rhs_array.size() < n)) return false; // Not enough elements for a true outcome.

    size_t matches = 0;
    for (size_t src_it = 0; src_it < rhs_array.size(); src_it++) {
        ps::string cur_str = rhs_array.at(src_it);

        for (size_t it = 0; it < lhs_array.size(); it++) {
            if (cur_str == lhs_array.at(it)) {
                matches++;
                if (matches >= n) return true; // Stop searching after enough matches are found.
                break;
            }
        }

    }

    return false;
}

/**
 * @brief Checks whether any elements of both arrays match.
 * @return True if any element is common between both arrays, else false.
*/
const bool Expression::arraySubsetComparison(const ps::vector<ps::string>& lhs_array, const ps::vector<ps::string>& rhs_array) const {
    return arrayMinQuantifierSearch(lhs_array, rhs_array, 1);
}

/**
 * @brief Checks whether the provided arrays match. Element order does not matter.
 * @return True if the two arrays match, else false.
*/
const bool Expression::arrayEqualityComparison(const ps::vector<ps::string>& lhs_array, const ps::vector<ps::string>& rhs_array) const {
    if (lhs_array.size() != rhs_array.size()) return false;
    return arrayMinQuantifierSearch(lhs_array, rhs_array, lhs_array.size());
}

ps::vector<ps::string> Expression::separateArray(const Token& token) {
    #ifdef DEBUG_RULE_ENGINE
    ESP_LOGD(TAG_RULE_ENGINE, "Separating Array...");
    uint64_t start_time = esp_timer_get_time();
    #endif

    if (token.type != ARRAY) throw std::invalid_argument("Not an ARRAY type token.");
    Lexer lexer(token.lexeme);
    ps::queue<Token> array_tokens = lexer.tokenize();

    ps::vector<ps::string> resultant_array;

    TokenType token_type;
    while (!array_tokens.empty())
    {
        token_type = array_tokens.front().type;

        if(token_type == STRING_LITERAL) resultant_array.push_back(array_tokens.front().lexeme);
        else if(token_type != SEPARATOR) throw std::invalid_argument("Only string literals are implemented in arrays at the moment.");
        
        array_tokens.pop();
    }

    #ifdef DEBUG_RULE_ENGINE
    ESP_LOGD(TAG_RULE_ENGINE, "Array Separated. [Took %uus]", esp_timer_get_time() - start_time);
    #endif

    return resultant_array;
}

Token Expression::combineArray(ps::vector<ps::string>& array) {
    Token ret;
    ret.type = ARRAY;
    ps::ostringstream output;
    for (size_t i = 0; i < array.size(); i++) {
        output << "\"" << array.at(i) << "\"";
        if (i != array.size() - 1) output << ",";
    }

    ret.lexeme = output.str();
    return ret;
}

} // namespace re