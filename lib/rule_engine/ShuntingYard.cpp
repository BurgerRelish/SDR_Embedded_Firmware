#include "ShuntingYard.h"
#ifdef DEBUG_RULE_ENGINE
#include "esp_timer.h"
#include "esp32-hal-log.h"
#define TAG_RULE_ENGINE "RULE_ENGINE"
#endif

ps_queue<Token> ShuntingYard::apply(ps_queue<Token>& tokenQueue) {

    #ifdef DEBUG_RULE_ENGINE
        uint64_t start_time = esp_timer_get_time();
    #endif
    ps_queue<Token> outputQueue;
    ps_stack<Token> operatorStack;

    while (!tokenQueue.empty()) {
        Token token = tokenQueue.front();
        tokenQueue.pop();

        switch (token.type) {
            case LEFT_PARENTHESES:
                operatorStack.push(token);
                break;

            case RIGHT_PARENTHESES:
                while (!operatorStack.empty() && operatorStack.top().type != LEFT_PARENTHESES) {
                    outputQueue.push(operatorStack.top());
                    operatorStack.pop();
                }

                if (operatorStack.empty()) {
                    // Mismatched parentheses error 
                    throw std::invalid_argument("Mismatched parentheses");
                } else {
                    operatorStack.pop(); // Discard the '('
                }
                break;

            case ARITHMETIC_OPERATOR:
            case BOOLEAN_OPERATOR:
            case COMPARISON_OPERATOR:
                while (!operatorStack.empty() && hasHigherPrecedence(operatorStack.top(), token)) {
                    outputQueue.push(operatorStack.top());
                    operatorStack.pop();
                }
                operatorStack.push(token);
                break;

            case SEPARATOR:
            break;

            default:
                outputQueue.push(token); // Output literals and identifiers directly
                break;
        }
    }

    while (!operatorStack.empty()) {
        if (operatorStack.top().type == LEFT_PARENTHESES) {
            // Mismatched parentheses error
            throw std::invalid_argument("Mismatched parentheses");
            break;
        }

        outputQueue.push(operatorStack.top());
        operatorStack.pop();
    }

    #ifdef DEBUG_RULE_ENGINE
    uint64_t tot_time = esp_timer_get_time() - start_time;

    ps_string debug;
    ps_queue<Token> debugQueue = outputQueue;
    while(!debugQueue.empty()) {
        debug += debugQueue.front().lexeme;
        debug += " ";
        debugQueue.pop();
    }
  
    ESP_LOGD(TAG_RULE_ENGINE, "\n==== Shunting Yard Analysis Complete ====");
    log_printf("- Processing Time: %uus\n", tot_time);
    log_printf("- Tokens: \'%s\'\n", debug.c_str());
    log_printf("=========================================\r\n");
    #endif

    return outputQueue;
}