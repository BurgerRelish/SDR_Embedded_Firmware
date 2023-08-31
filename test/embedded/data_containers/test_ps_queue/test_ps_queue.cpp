#include <Arduino.h>
#include <unity.h>
#include "../ps_stl/ps_stl.h"
#include <queue>

void testQueueFunctionality() {
    ps::queue<int> queue1;
    std::queue<int> queue2;

    // Test empty queues
    TEST_ASSERT_TRUE(queue1.empty());
    TEST_ASSERT_EQUAL_UINT32(0, queue1.size());

    // Test pushing elements
    queue1.push(1);
    queue1.push(2);
    queue1.push(3);

    TEST_ASSERT_FALSE(queue1.empty());
    TEST_ASSERT_EQUAL_UINT32(3, queue1.size());
    TEST_ASSERT_EQUAL_INT(1, queue1.front());

    // Test copying queues
    queue2 <<= queue1;

    TEST_ASSERT_EQUAL_UINT32(3, queue2.size());
    TEST_ASSERT_TRUE(queue1 == queue2);

    queue1 <<= queue2;

    TEST_ASSERT_EQUAL_UINT32(3, queue1.size());
    TEST_ASSERT_TRUE(queue1 == queue2);


    // Test popping elements
    queue2.pop();
    TEST_ASSERT_EQUAL_UINT32(2, queue2.size());
    TEST_ASSERT_EQUAL_INT(2, queue2.front());

    queue2.pop();
    TEST_ASSERT_EQUAL_UINT32(1, queue2.size());
    TEST_ASSERT_EQUAL_INT(3, queue2.front());

    queue2.pop();
    TEST_ASSERT_EQUAL_UINT32(0, queue2.size());
    TEST_ASSERT_TRUE(queue2.empty());

    // Test comparison operators
    std::queue<int> queue3;
    queue3.push(4);
    queue3.push(5);

    TEST_ASSERT_FALSE(queue1 == queue3);
    TEST_ASSERT_TRUE(queue1 != queue3);
    TEST_ASSERT_TRUE(queue3 != queue1);
}


void setup() {
    delay(2000);
    psramInit();

    UNITY_BEGIN();
    RUN_TEST(testQueueFunctionality);
    UNITY_END();
}

void loop() {
}