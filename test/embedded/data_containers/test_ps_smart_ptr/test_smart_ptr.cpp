#include <Arduino.h>
#include <unity.h>
#include "../data_containers/ps_smart_ptr.h"

void test_make_shared() {
    int value = 42;
    auto sharedPtr = ps::make_shared<int>(value);

    TEST_ASSERT_EQUAL_INT(value, *sharedPtr);
}

void test_make_unique() {
    int value = 42;
    auto uniquePtr = ps::make_unique<int>(value);

    TEST_ASSERT_EQUAL_INT(value, *uniquePtr);
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_make_shared);
    RUN_TEST(test_make_unique);
    UNITY_END();
}

void loop() {
    // Nothing to do here for unit tests
}