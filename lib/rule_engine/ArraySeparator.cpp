#include "ArraySeparator.h"

ps_vector<ps_string> ArraySeparator::separate(const Token& token) {
    if (token.type != ARRAY) throw std::invalid_argument("Not an ARRAY type token.");

    ps_queue<Token> array_tokens = tokenize_array(token.lexeme);

    ps_vector<ps_string> resultant_array;

    TokenType token_type;
    while (!array_tokens.empty())
    {
        token_type = array_tokens.front().type;

        if(token_type == STRING_LITERAL) resultant_array.push_back(array_tokens.front().lexeme);
        if(token_type != SEPARATOR) throw std::invalid_argument("Only string literals are implemented in arrays at the moment.");
        
        array_tokens.pop();
    }

    return resultant_array;
}

ps_queue<Token> ArraySeparator::tokenize_array(const ps_string& string) {
    Lexer lexer(string);
    return lexer.tokenize();
}