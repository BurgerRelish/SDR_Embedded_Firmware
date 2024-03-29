#include "Lexer.h"
#ifdef DEBUG_RULE_ENGINE
#include "esp_timer.h"
#include "esp32-hal-log.h"
#define TAG_RULE_ENGINE "RULE_ENGINE"
#endif

#include "Semantics.h"

ps::queue<Token> Lexer::tokenize() {
    #ifdef DEBUG_RULE_ENGINE
    ESP_LOGD(TAG_RULE_ENGINE, "Lexer started.");
    uint64_t start_time = esp_timer_get_time();
    #endif

    size_t expr_size = expression_.size();
    ps::queue<Token> token_list;
    char cur_char;

    while (index < expr_size) {
        cur_char = expression_[index];

        if (isWhitespace(cur_char)) {
            index++;
        } else if (isArithmeticOperator(cur_char)) {
            token_list.push(handleArithmeticOperator());
        } else if (isBooleanOperator(cur_char)) {
            token_list.push(handleBooleanOperator());
        } else if (isArray(cur_char)) {
            token_list.push(handleArray());
        } else if (isStringLiteral(cur_char)) {
            token_list.push(handleStringLiteral());
        } else if (isNumericLiteral(cur_char)) {
            token_list.push(handleNumericLiteral());
        } else if (isParenthesis(cur_char)) {
            token_list.push(handleParenthesis());
        } else if (isComparisonOperator(cur_char)) {
            token_list.push(handleComparisonOperator());
        } else if(isalnum(cur_char) || cur_char == '_'){
            token_list.push(handleIdentifier());
        } else if (isSeparator(cur_char)) {
            token_list.push(handleSeparator());
        }else {
            throw std::invalid_argument("Unknown value in expression.");
        }
    }

    #ifdef DEBUG_RULE_ENGINE
    uint64_t tot_time = esp_timer_get_time() - start_time;
    
    ps::string debug;
    ps::queue<Token> debugQueue = token_list;

    while(!debugQueue.empty()) {
        debug += debugQueue.front().lexeme;
        debug += ps::string(" ");
        debugQueue.pop();
    }
    ESP_LOGD(TAG_RULE_ENGINE, "\n==== Lexical Analysis Complete ====");
    log_printf("- Processing Time: %uus\n", tot_time);
    log_printf("- Expression: \'%s\'\n", expression_.c_str());
    log_printf("- Tokens: \'%s\'\n", debug.c_str());
    log_printf("===================================\r\n");
    
    #endif

    return token_list;
}

bool Lexer::isWhitespace(const char ch) const{
    return (ch == ' ' || ch == '\r' || ch == '\t' || ch == '\n');
}


bool Lexer::isArithmeticOperator(const char ch) const{
    return (ch == ARITHMETIC_ADD[0] || ch == ARITHMETIC_SUBTRACT[0] || ch == ARITHMETIC_DIVIDE[0]
            || ch == ARITHMETIC_MULTIPLY[0] || ch == ARITHMETIC_POWER[0] || ch == ARITHMETIC_MODULUS[0]);
}


bool Lexer::isBooleanOperator(const char ch) const{
    return (ch == BOOLEAN_AND[0] || ch == BOOLEAN_NOT[0] || ch == BOOLEAN_OR[0]);
}

bool Lexer::isComparisonOperator(const char ch) const {
    return (ch == COMPARISON_EQUAL[0] || ch == COMPARISON_GREATER_THAN[0] || ch == COMPARISON_LESSER_THAN[0] || ch == COMPARISON_NOT_EQUAL[0]);
}


bool Lexer::isStringLiteral(const char ch) const{
    return ch == STRING_LITERAL_QUOTATION[0];
}


bool Lexer::isNumericLiteral(const char ch) const{
    return (isdigit(ch) || ch == '.');
}


bool Lexer::isParenthesis(const char ch) const{
    return (ch == '(' || ch == ')');
}

bool Lexer::isArray(const char ch) const {
    return ch == '[';
}

bool Lexer::isSeparator(const char ch) const {
    return (ch == ARRAY_SEPARATOR[0] || ch == COMMAND_SEPARATOR[0]);
}

Token Lexer::handleBooleanOperator() {
    Token token;

    token.type = BOOLEAN_OPERATOR;

    char cur_char = expression_[index];
    char next_char;

    if (index + 1 < expression_.size()) {
        next_char = expression_[index + 1];
    } else {
        next_char = '\0';
    }

    switch (cur_char)
    {
    case '&':
        if(next_char == '&') {
            token.lexeme = "&&";
            index += 2;
        } else {
            throw std::invalid_argument("Invalid boolean operator.");
        }
        break;
    case '|':
        if(next_char == '|') {
            token.lexeme = "||";
            index += 2;
        } else {
            throw std::invalid_argument("Invalid boolean operator.");
        }
        break;
    case '!':
        if (next_char == '!' || isWhitespace(next_char) || isalnum(next_char) || next_char == '_' || next_char == '(') {
            token.lexeme = "!";
            index++;
        } else if (next_char == '=') {
            return handleComparisonOperator(); // Handle !=
        } else {
            throw std::invalid_argument("Invalid boolean operator.");
        }
        break;
    }


    return token;
}


Token Lexer::handleArithmeticOperator() {
    Token token;

    token.type = ARITHMETIC_OPERATOR;
    token.lexeme = expression_[index];

    if(!(index + 1 < expression_.size())) {
        throw std::invalid_argument("Invalid arithmetic operator.");
    }

    index++;

    return token;
}


Token Lexer::handleStringLiteral() {
    index++; // Skip opening quotation

    Token token;
    token.type = STRING_LITERAL;

    while (!isStringLiteral(expression_[index])) {
        token.lexeme += expression_[index];
        index++;
        if (index == expression_.size()) break;
    }

    index++;

    if (index > expression_.size() ) throw std::invalid_argument("String literal was not closed.");

    return token;
}


Token Lexer::handleNumericLiteral() {
    Token token;
    token.type = NUMERIC_LITERAL;

    while (isNumericLiteral(expression_[index]) && index < expression_.size()) {
        token.lexeme += expression_.at(index);
        index++;
    }

    return token;
}


Token Lexer::handleIdentifier() {
    Token token;
    token.type = IDENTIFIER;

    while ((isalnum(expression_[index]) || expression_[index] == '_') && index < expression_.size()) {
        token.lexeme += expression_[index];
        index++;
    }

    return token;
}


Token Lexer::handleParenthesis() {
    Token token;

    switch(expression_[index]) {
        case '(':
        token.type = LEFT_PARENTHESES;
        break;
        case ')':
        token.type = RIGHT_PARENTHESES;
        break;
        default:
            throw std::invalid_argument("Invalid Parenthesis.");
        break;
    }

    token.lexeme = expression_[index];

    index++;
    return token;
}

Token Lexer::handleArray() {
    index++; // Skip opening bracket

    Token token;
    token.type = ARRAY;

    while (expression_[index] != ']') {
        token.lexeme += expression_[index];
        index++;
        if (index == expression_.size()) break;
    }

    index ++; // Skip closing bracket.

    if (index > expression_.size()) throw std::invalid_argument("Array was not closed.");

    return token;
}

Token Lexer::handleComparisonOperator() {
    Token token;

    token.type = COMPARISON_OPERATOR;

    char cur_char = expression_[index];
    char next_char;

    if (index + 1 < expression_.size()) {
        next_char = expression_[++index];
    } else {
        throw std::invalid_argument("Invalid comparison operator.");
    }
    
    switch (cur_char) {
        case '<':
            if (next_char == '=') {
                token.lexeme = "<=";
                index++;
                break;
            }

            token.lexeme = "<";
            break;
        case '>':
            if (next_char == '=') {
                token.lexeme = ">=";
                index++;
                break;
            }

            token.lexeme = ">";
            break;
        case '=':
            if (next_char != '=') throw std::invalid_argument("Invalid comparison operator.");
            token.lexeme = "==";
            index++;
            break;
        case '!':
            if (next_char != '=') throw std::invalid_argument("Invalid comparison operator.");
            token.lexeme = "!=";
            index++;
            break;
    }

    return token;
}

Token Lexer::handleSeparator() {
    Token token;

    
    token.type = SEPARATOR;
    token.lexeme += (expression_[index]);
    index++;

    return token;
}

