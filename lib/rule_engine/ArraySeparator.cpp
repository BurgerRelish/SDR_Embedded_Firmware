#include "ArraySeparator.h"

#ifdef DEBUG_RULE_ENGINE
#include "esp_timer.h"
#include "esp32-hal-log.h"
#define TAG_RULE_ENGINE "RULE_ENGINE"
#endif

ps::vector<ps::string> ArraySeparator::separate(const Token& token) {
    #ifdef DEBUG_RULE_ENGINE
    ESP_LOGD(TAG_RULE_ENGINE, "Separating Array...");
    uint64_t start_time = esp_timer_get_time();
    #endif

    if (token.type != ARRAY) throw std::invalid_argument("Not an ARRAY type token.");

    ps::queue<Token> array_tokens = tokenize_array(token.lexeme);

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

ps::queue<Token> ArraySeparator::tokenize_array(const ps::string& string) {
    Lexer lexer(string);
    return lexer.tokenize();
}