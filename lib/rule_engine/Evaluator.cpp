#include "Evaluator.h"

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

        rules.push(new_rule);
    }   
}

ps_queue<Token> Evaluator::lexExpression(ps_string& expr) {
    Lexer lexer(expr);
    return lexer.tokenize();
}

bool Evaluator::evaluateRPN(ps_queue<Token>& tokens) {
    ps_queue<Token> token_list = tokens;
    Token current_token;
    Token operand_1;
    Token operand_2;
    bool ret_val = false;
    
    while (!token_list.empty()) {
        current_token = token_list.front();

        switch(current_token.type) {
            case ARRAY: // ]
                break;
            case ARITHMETIC_OPERATOR: // + - / ^ %
                break;
            case BOOLEAN_OPERATOR: // && || !
                break;
            case COMPARISON_OPERATOR: // == != <= >= < >
                break;
            case STRING_LITERAL: // "Hello World!"
                break;
            case NUMERIC_LITERAL: // 1 1.201
                break;
            case IDENTIFIER:
                break;
            case SEPARATOR: // ,
                break;
            default:
                break;
        }

        token_list.pop();
    }
}

ps_queue<ps_string> Evaluator::separateArray(Token& array) {
    ArraySeparator separator;
    return separator.separate(array);
}