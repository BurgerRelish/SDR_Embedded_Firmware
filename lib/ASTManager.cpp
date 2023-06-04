#include "ASTManager.h"

const char* arithmetic_operators = "+-*/=";
const char* logical_operators = "&!|";
const char* separators = "()[]{};";

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
    char character = command -> at(*pointer);

    if(isspace(character)){ // If it is whitespace, go to the next character
        *pointer++;
        return last_token;
    }

    struct Token* new_token = (struct Token*)ps_malloc(sizeof(struct Token));
    if(last_token != nullptr) last_token -> next_token = new_token; // Only store the new token address if its not the first token.

    if(isalpha(character)){ // Check if it is an identifier/keyword

    } else if (isdigit(character)){ // Check if it is a number

    } else if (character == '\"') { // Check if string literal

    } else { // Else it is a separator

    }

}

bool Lexer::isin(char* character, const char* array){
    for(int i = 0; i < sizeof(array), i++){
        if(*character == *(array + i)) return true;
    }

    return false;
}