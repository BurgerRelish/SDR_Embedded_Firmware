#include <unity.h>
#include "ps_string.h"
#include "ps_queue.h.h"

void test_stringLength() {
  ps::string str = "Hello, World!";
  TEST_ASSERT_EQUAL(13, str.length());
}

void test_stringConcatenation() {
  ps::string str1 = "Hello, ";
  ps::string str2 = "World!";
  ps::string result = str1 + str2;
  TEST_ASSERT_EQUAL_STRING("Hello, World!", result.c_str());
}

void test_std_into_ps() {
    std::string std_string = "Hello World!";

    ps::string new_string; 
    new_string <<= std_string;

    TEST_ASSERT_TRUE(new_string == std_string);

    std::string new_std_string;
    new_std_string <<= new_string;

    TEST_ASSERT_TRUE(new_std_string == std_string);

}

ps::queue<ps::string> ret_str_in_queue() {
  ps::queue<ps::string> ret;

  ret.push("Hello");
  ret.push("World!");

  return ret;
}

void test_string_in_queue() {
    ps::queue<ps::string> queue;
    ps::string string = "Hello1";
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