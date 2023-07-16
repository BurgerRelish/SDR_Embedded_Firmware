#include <Arduino.h>

#define UNITY_INCLUDE_DOUBLE
#include <unity.h>

#include "../sdr_containers.h"
#include <vector>
#include <string>

void test_unit_container() {
    const std::string id = "testID123";
    const std::vector<std::string> tag_list = {"Hello", "World", "tag3"};
    SDRUnit container(id, 2, tag_list);

    container.powerStatus() = true;
    TEST_ASSERT_TRUE(container.powerStatus());
    container.powerStatus() = false;
    TEST_ASSERT_FALSE(container.powerStatus());

    container.totalActivePower() = 5.00;
    TEST_ASSERT_EQUAL_DOUBLE(5.00, container.totalActivePower());
    container.totalActivePower() = 12.670;
    TEST_ASSERT_EQUAL_DOUBLE(12.670, container.totalActivePower());

    ps_vector<ps_string> tag_search = {"Hello", "World", "tag3"};
    TEST_ASSERT_TRUE(container.tagEqualityComparison(tag_search));
    tag_search.pop_back();
    TEST_ASSERT_FALSE(container.tagEqualityComparison(tag_search));
    TEST_ASSERT_TRUE(container.tagSubsetComparison(tag_search));
    TEST_ASSERT_FALSE(container.tagSubsetComparison(ps_string("Test2")));
}

void setup() {
    psramInit();
    delay(2000);

    UNITY_BEGIN();

    RUN_TEST(test_unit_container);

    UNITY_END();
}

void loop() {}