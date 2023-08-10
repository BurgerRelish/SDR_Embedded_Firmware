#include <Arduino.h>

#define UNITY_INCLUDE_DOUBLE
#include <unity.h>
#include "unity_internals.h"

#include "../sdr_containers.h"
#include "ps_string.h"
#include "ps_vector.h"
#include "ps_stack.h.h"


void test_module_container() {
    const std::string id = "testID123";
    const std::vector<std::string> tag_list = {"Hello", "World", "tag3"};
    const int priority = 5;
    const uint8_t address = 10;
    const uint8_t offset = 2;

    const Reading test_reading_0 = {
        230.1, // double voltage;
        50.5, // double frequency;
        123.1, // double active_power;
        456.2, // double reactive_power;
        789.1, // double apparent_power;
        1.232, // double power_factor;
        true, // bool status;
        10000 // uint64_t timestamp;
    };

    ps::vector<ps::string> tag_search = {"Hello", "World", "tag3"};

    Module container(id, tag_list, priority, address, offset);
    container.addReading(test_reading_0);
    TEST_ASSERT_EQUAL_INT(5, container.priority());
    TEST_ASSERT_EQUAL_INT(10, container.address());
    TEST_ASSERT_EQUAL_INT(2, container.offset());
    TEST_ASSERT_TRUE(container.status());
    TEST_ASSERT_EQUAL_INT(10000, container.switchTime());

    TEST_ASSERT_TRUE(container.tagEqualityComparison(tag_search));
    TEST_ASSERT_TRUE(container.tagSubsetComparison(tag_search));
    tag_search.pop_back();
    TEST_ASSERT_FALSE(container.tagEqualityComparison(tag_search));
    TEST_ASSERT_TRUE(container.tagSubsetComparison(tag_search));
    tag_search.erase(tag_search.begin());
    TEST_ASSERT_FALSE(container.tagEqualityComparison(tag_search));
    TEST_ASSERT_TRUE(container.tagSubsetComparison(tag_search));

    ps::string search_str = "tag3";
    TEST_ASSERT_TRUE(container.tagSubsetComparison(search_str));


    TEST_ASSERT_EQUAL_DOUBLE(test_reading_0.active_power, container.latestReading().active_power);

    ps::stack<Reading> reading_stack;
    reading_stack = container.getReadings();

    TEST_ASSERT_EQUAL_INT(1, reading_stack.size());
    TEST_ASSERT_EQUAL_DOUBLE(test_reading_0.active_power, reading_stack.top().active_power);
}

void setup() {
    psramInit();
    delay(2000);

    UNITY_BEGIN();

    RUN_TEST(test_module_container);

    UNITY_END();
}

void loop() {}