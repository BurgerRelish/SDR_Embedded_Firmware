#include "ASTManager.h"

struct Token* Lexer::lex(std::string* command){
    uint32_t pointer = 0;
    size_t command_length = command -> length();

    struct Token* last_token = nullptr;
    struct Token* ret = generateToken(command, &pointer, last_token);
    last_token = ret;

    while(pointer < command_length){
        last_token = generateToken(command, &pointer, last_token);
    }

    return ret;
}

struct Token* Lexer::generateToken(std::string* command, uint32_t* pointer, struct Token* last_token){
    struct Token* new_token = (struct Token*)ps_malloc(sizeof(struct Token));
    last_token -> next_token = new_token;

    
}