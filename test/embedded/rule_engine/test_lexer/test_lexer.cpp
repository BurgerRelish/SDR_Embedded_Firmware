#include <Arduino.h>
#include <unity.h>
#include "Lexer.h"

#include "ps_string.h"
#include "ps_queue.h.h"

void testTokenize() {
    
    std::string expression = "(3.14 + 2) * 5 (\"TEST\") || 1";
    Lexer lexer(expression);
    ps::queue<Token> tokens = lexer.tokenize();

    TEST_ASSERT_EQUAL_INT(LEFT_PARENTHESES, tokens.front().type);
    TEST_ASSERT_EQUAL_STRING("(", tokens.front().lexeme.c_str());
    tokens.pop();

    TEST_ASSERT_EQUAL_INT(NUMERIC_LITERAL, tokens.front().type);
    TEST_ASSERT_EQUAL_STRING("3.14", tokens.front().lexeme.c_str());
    tokens.pop();

    TEST_ASSERT_EQUAL_INT(ARITHMETIC_OPERATOR, tokens.front().type);
    TEST_ASSERT_EQUAL_STRING("+", tokens.front().lexeme.c_str());
    tokens.pop();

    TEST_ASSERT_EQUAL_INT(NUMERIC_LITERAL, tokens.front().type);
    TEST_ASSERT_EQUAL_STRING("2", tokens.front().lexeme.c_str());
    tokens.pop();

    TEST_ASSERT_EQUAL_INT(RIGHT_PARENTHESES, tokens.front().type);
    TEST_ASSERT_EQUAL_STRING(")", tokens.front().lexeme.c_str());
    tokens.pop();

    TEST_ASSERT_EQUAL_INT(ARITHMETIC_OPERATOR, tokens.front().type);
    TEST_ASSERT_EQUAL_STRING("*", tokens.front().lexeme.c_str());
    tokens.pop();

    TEST_ASSERT_EQUAL_INT(NUMERIC_LITERAL, tokens.front().type);
    TEST_ASSERT_EQUAL_STRING("5", tokens.front().lexeme.c_str());
    tokens.pop();

    TEST_ASSERT_EQUAL_INT(LEFT_PARENTHESES, tokens.front().type);
    TEST_ASSERT_EQUAL_STRING("(", tokens.front().lexeme.c_str());
    tokens.pop();

    TEST_ASSERT_EQUAL_INT(STRING_LITERAL, tokens.front().type);
    TEST_ASSERT_EQUAL_STRING("TEST", tokens.front().lexeme.c_str());
    tokens.pop();

    TEST_ASSERT_EQUAL_INT(RIGHT_PARENTHESES, tokens.front().type);
    TEST_ASSERT_EQUAL_STRING(")", tokens.front().lexeme.c_str());
    tokens.pop();

    TEST_ASSERT_EQUAL_INT(BOOLEAN_OPERATOR, tokens.front().type);
    TEST_ASSERT_EQUAL_STRING("||", tokens.front().lexeme.c_str());
    tokens.pop();
    tokens.pop();
    TEST_ASSERT_TRUE(tokens.empty());
}

void setup() {
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(testTokenize);
    UNITY_END();
}

void loop() {
}