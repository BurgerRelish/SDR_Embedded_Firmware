#ifndef AST_MANAGER_H
#define AST_MANAGER_H
#include <Arduino.h>
#include "../DataStructures.h"
#include <string>

enum TokenType{KEYWORD, IDENTIFIER, ARITHMETIC_OPERATOR, LOGICAL_OPERATOR, LITERAL, SEPARATOR, INVALID};
struct Token{
    struct Token* next_token;
    enum TokenType type;
    std::string* value;
};

class Lexer{
    public:
       struct Token* lex(std::string* command); 
    private:
        struct Token* generateToken(std::string* command, uint32_t* pointer, struct Token* last_token);
        std::string* getKeyword(std::string* command, uint32_t* pointer);
        std::string* getIdentifier(std::string* command, uint32_t* pointer);
        std::string* getOperator(std::string* command, uint32_t* pointer);
        std::string* getLiteral(std::string* command, uint32_t* pointer);
        std::string* getSeparator(std::string* command, uint32_t* pointer);

        bool isin(char* character, const char* array);
};

class Parser{
    public:
        void parse();

};

class Interpreter{

};

class ASTManager: private Lexer, private Parser, private Interpreter{
    public:
        ASTManager(struct ASTNode* start_node);
        void process(struct ASTQueueMessage* command);
        void interpret(struct ModuleMetaData* module_metadata_tree);
        void interpret();
    private:
        struct ASTNode* AST = nullptr;
};

#endif