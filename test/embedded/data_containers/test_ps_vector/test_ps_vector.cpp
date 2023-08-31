#include <Arduino.h>
#include <unity.h>
#include "../ps_stl/ps_stl.h"

void test_vector_operations(void) {
  // Arrange
  
  ps::vector<int> vec;

  // Act
  vec.push_back(10);
  vec.push_back(20);
  vec.push_back(30);

  // Assert
  TEST_ASSERT_EQUAL_INT(3, vec.size());
  TEST_ASSERT_EQUAL_INT(10, vec[0]);
  TEST_ASSERT_EQUAL_INT(20, vec[1]);
  TEST_ASSERT_EQUAL_INT(30, vec[2]);

  // Copying
  std::vector<int> vec2;
  vec2 <<= vec;

  TEST_ASSERT_EQUAL_INT(3, vec2.size());
  TEST_ASSERT_EQUAL_INT(10, vec2[0]);
  TEST_ASSERT_EQUAL_INT(20, vec2[1]);
  TEST_ASSERT_EQUAL_INT(30, vec2[2]);

  ps::vector<int> vec3;
  vec3 <<= vec2;

  TEST_ASSERT_EQUAL_INT(3, vec3.size());
  TEST_ASSERT_EQUAL_INT(10, vec3[0]);
  TEST_ASSERT_EQUAL_INT(20, vec3[1]);
  TEST_ASSERT_EQUAL_INT(30, vec3[2]);

  // Act
  vec.pop_back();

  // Assert
  TEST_ASSERT_EQUAL_INT(2, vec.size());
  TEST_ASSERT_EQUAL_INT(10, vec[0]);
  TEST_ASSERT_EQUAL_INT(20, vec[1]);

  // Act
  vec.clear();

  // Assert
  TEST_ASSERT_EQUAL_INT(0, vec.size());
  TEST_ASSERT_TRUE(vec.empty());
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_vector_operations);
  UNITY_END();
}

void loop() {
  // Empty loop
}