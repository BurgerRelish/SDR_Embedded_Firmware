#include <Arduino.h>
#include <unity.h>
#include "../ps_stl/ps_stl.h"

void test_make_shared() {
    int value = 42;
    auto sharedPtr = ps::make_shared<int>(value);

    TEST_ASSERT_EQUAL_INT(value, *sharedPtr);
}


void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_make_shared);
    UNITY_END();
}

void loop() {
    // Nothing to do here for unit tests
}