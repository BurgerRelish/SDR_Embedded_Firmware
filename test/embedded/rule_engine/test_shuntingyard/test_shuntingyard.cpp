#include <Arduino.h>
#include <unity.h>
#include "ps_queue.h.h"
#include "ps_string.h"
#include "Language.h"
#include "ShuntingYard.h"
#include "Lexer.h"

void testShuntingYard() {
    // Input queue of tokens
    
    std::string inputstr = "((3 + 4) <= test_var) && test_array == [\"hello world\", \"test123\", \"...\"]";
    Lexer lexer(inputstr);
    ps::queue<Token> inputQueue;

    try {
        inputQueue = lexer.tokenize(); 
    } catch (std::exception &e) {
        TEST_ASSERT_TRUE_MESSAGE(false, e.what());
        return;
    }
    // Expected output queue of tokens
    // 3 4 + test_var <= test_array ["hello world"] == &&
    ps::queue<Token> expectedOutputQueue;
    expectedOutputQueue.push({NUMERIC_LITERAL, "3"});
    expectedOutputQueue.push({NUMERIC_LITERAL, "4"});
    expectedOutputQueue.push({ARITHMETIC_OPERATOR, "+"});
    expectedOutputQueue.push({IDENTIFIER, "test_var"});
    expectedOutputQueue.push({COMPARISON_OPERATOR, "<="});
    expectedOutputQueue.push({IDENTIFIER, "test_array"});
    expectedOutputQueue.push({ARRAY, "\"hello world\", \"test123\", \"...\""});
    expectedOutputQueue.push({COMPARISON_OPERATOR, "=="});
    expectedOutputQueue.push({BOOLEAN_OPERATOR, "&&"});

    ShuntingYard shuntingYard;
    ps::queue<Token> actualOutputQueue;
    try {
        actualOutputQueue = shuntingYard.apply(inputQueue);
    } catch (std::exception &e) {
        TEST_ASSERT_TRUE_MESSAGE(false, e.what());
        return;
    }
    

    // Compare the actual and expected output queues
    while (!expectedOutputQueue.empty()) {
        TEST_ASSERT_EQUAL(expectedOutputQueue.front().type, actualOutputQueue.front().type);
        TEST_ASSERT_EQUAL_STRING(expectedOutputQueue.front().lexeme.c_str(), actualOutputQueue.front().lexeme.c_str());
        expectedOutputQueue.pop();
        actualOutputQueue.pop();
    }

    TEST_ASSERT_TRUE(actualOutputQueue.empty());
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(testShuntingYard);
    UNITY_END();
}

void loop() {
    // Your code here
}