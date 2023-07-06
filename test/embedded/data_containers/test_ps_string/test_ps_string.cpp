#include <unity.h>
#include "ps_string.h"
#include "ps_queue.h"

void test_stringLength() {
  ps_string str = "Hello, World!";
  TEST_ASSERT_EQUAL(13, str.length());
}

void test_stringConcatenation() {
  ps_string str1 = "Hello, ";
  ps_string str2 = "World!";
  ps_string result = str1 + str2;
  TEST_ASSERT_EQUAL_STRING("Hello, World!", result.c_str());
}

void test_std_into_ps() {
    std::string std_string = "Hello World!";

    ps_string new_string; 
    new_string <<= std_string;

    TEST_ASSERT_TRUE(new_string == std_string);

    std::string new_std_string;
    new_std_string <<= new_string;

    TEST_ASSERT_TRUE(new_std_string == std_string);

}

ps_queue<ps_string> ret_str_in_queue() {
  ps_queue<ps_string> ret;

  ret.push("Hello");
  ret.push("World!");

  return ret;
}

void test_string_in_queue() {
    ps_queue<ps_string> queue;
    ps_string string = "Hello1";
    queue.push(string);

    string = "World!2";
    queue.push(string);

    string = queue.front();

    TEST_ASSERT_TRUE(string == "Hello1");
    queue.pop();

    string = queue.front();

    TEST_ASSERT_TRUE(string == "World!2");
}

void test_ret_str_in_queue() {
  auto ret = ret_str_in_queue();

  TEST_ASSERT_TRUE(ret.front() == "Hello");
  ret.pop();

  TEST_ASSERT_TRUE(ret.front() == "World!");

}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_stringLength);
  RUN_TEST(test_stringConcatenation);
  RUN_TEST(test_string_in_queue);
  RUN_TEST(test_std_into_ps);
  RUN_TEST(test_ret_str_in_queue);
  UNITY_END();
}

void loop() {
  // Empty loop
}