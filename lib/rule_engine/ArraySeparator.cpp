#include "ArraySeparator.h"

ps_queue<ps_string> ArraySeparator::separate(const Token& token) {
    if (token.type != ARRAY) throw std::invalid_argument("Not an ARRAY type token.");

    ps_queue<Token> array_tokens = tokenize_array(token.lexeme);

    ps_queue<ps_string> separated_queue;

    TokenType token_type;
    while (!array_tokens.empty())
    {
        token_type = array_tokens.front().type;

        if(token_type != STRING_LITERAL || token_type != SEPARATOR) throw std::invalid_argument("Only string literals are implemented in arrays at the moment.");
        if(token_type == STRING_LITERAL) separated_queue.push(array_tokens.front().lexeme);

        separated_queue.pop();
    }

    return separated_queue;
}

ps_queue<Token> ArraySeparator::tokenize_array(const ps_string& string) {
    Lexer lexer(string);
    return lexer.tokenize();
}