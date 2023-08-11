#include <Arduino.h>

#include <unity.h>
#include <queue>
#include <any>
#include "../ps_stl/ps_stl.h"

void test_push_back()
{
    ps::deque<int> deque;
    deque.push_back(1);
    deque.push_back(2);
    deque.push_back(3);

    TEST_ASSERT_EQUAL(3, deque.size());
    TEST_ASSERT_EQUAL(1, deque.front());
    TEST_ASSERT_EQUAL(3, deque.back());
}

void test_push_front()
{
    ps::deque<int> deque;
    deque.push_front(3);
    deque.push_front(2);
    deque.push_front(1);

    TEST_ASSERT_EQUAL(3, deque.size());
    TEST_ASSERT_EQUAL(1, deque.front());
    TEST_ASSERT_EQUAL(3, deque.back());
}

void test_pop_front()
{
    ps::deque<int> deque;
    deque.push_back(1);
    deque.push_back(2);
    deque.push_back(3);

    deque.pop_front();

    TEST_ASSERT_EQUAL(2, deque.size());
    TEST_ASSERT_EQUAL(2, deque.front());
    TEST_ASSERT_EQUAL(3, deque.back());
}

void test_pop_back()
{
    ps::deque<int> deque;
    deque.push_back(1);
    deque.push_back(2);
    deque.push_back(3);

    deque.pop_back();

    TEST_ASSERT_EQUAL(2, deque.size());
    TEST_ASSERT_EQUAL(1, deque.front());
    TEST_ASSERT_EQUAL(2, deque.back());
}

void test_clear()
{
    ps::deque<int> deque;
    deque.push_back(1);
    deque.push_back(2);
    deque.push_back(3);

    deque.clear();

    TEST_ASSERT_EQUAL(0, deque.size());
    TEST_ASSERT_TRUE(deque.empty());
}

void test_at()
{
    ps::deque<int> deque;
    deque.push_back(1);
    deque.push_back(2);
    deque.push_back(3);

    TEST_ASSERT_EQUAL(2, deque.at(1));
    TEST_ASSERT_EQUAL(3, deque.at(2));
}

void test_erase()
{
    ps::deque<int> deque;
    deque.push_back(1);
    deque.push_back(2);
    deque.push_back(3);

    deque.erase(deque.begin() + 1);
    
    TEST_ASSERT_EQUAL(2, deque.size());
    TEST_ASSERT_EQUAL(1, deque.front());
    TEST_ASSERT_EQUAL(3, deque.back());
}

void test_empty()
{
    ps::deque<int> deque;
    TEST_ASSERT_TRUE(deque.empty());

    deque.push_back(1);
    TEST_ASSERT_FALSE(deque.empty());

    deque.pop_back();
    TEST_ASSERT_TRUE(deque.empty());
}

void test_size()
{
    ps::deque<int> deque;
    TEST_ASSERT_EQUAL(0, deque.size());

    deque.push_back(1);
    deque.push_back(2);
    deque.push_back(3);
    TEST_ASSERT_EQUAL(3, deque.size());

    deque.pop_front();
    TEST_ASSERT_EQUAL(2, deque.size());
}

bool isPsramAllocated(void* ptr) {
  return (reinterpret_cast<uintptr_t>(ptr) >= 0x3D000000);
}

void test_ps_deque_psram_allocation() {
  // Create a ps::deque of integers
  ps::deque<int> myDeque;

  // Add some elements to the deque
  myDeque.push_back(1);
  myDeque.push_back(2);
  myDeque.push_back(3);

  // Check if the memory is allocated in PSRAM
  for (const auto& num : myDeque) {
    TEST_ASSERT_TRUE_MESSAGE(isPsramAllocated((void*)&num), "Element in ps::deque is not allocated in PSRAM");
  }
}

void test_ps_deque_in_ps_deque() {
    ps::deque<int> deque;
    ps::deque<ps::deque<int>> nested_deque;

    deque.push_back(1);
    deque.push_back(2);
    deque.push_back(3);

    nested_deque.push_back(deque);
    deque.clear();

    deque.push_back(4);
    deque.push_back(5);
    deque.push_back(6);
    deque.push_back(7);

    nested_deque.push_back(deque);
    deque.clear();

    deque = nested_deque.front();

    TEST_ASSERT_EQUAL(3, deque.size());
    TEST_ASSERT_EQUAL(1, deque.front());
    TEST_ASSERT_EQUAL(3, deque.back());

    nested_deque.pop_front();
    deque = nested_deque.front();

    TEST_ASSERT_EQUAL(4, deque.size());
    TEST_ASSERT_EQUAL(4, deque.front());
    TEST_ASSERT_EQUAL(7, deque.back());

}


void test_queue_functionality() {
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

void test_make_shared() {
    int value = 42;
    auto sharedPtr = ps::make_shared<int>(value);

    TEST_ASSERT_EQUAL_INT(value, *sharedPtr);
}

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

void test_unordered_map() {
    ps::unordered_map<ps::string, int> store;

    store.insert(
        std::make_pair(
            ps::string("test123"),
            12345
        )
    );

    auto result = store.find("test123");
    if (result == store.end()){
        TEST_ASSERT_TRUE(false);
        return;
    } 
    TEST_ASSERT_EQUAL_INT(12345, result->second);
}

void test_std_any() {
    auto var = std::any(12345);
    int result = std::any_cast<int>(var);

    TEST_ASSERT_EQUAL_INT(12345, result);
}

void test_std_any_in_unordered_map() {
    ps::unordered_map<ps::string, std::any> store;
    store.insert(
        std::make_pair(
            ps::string("test"),
            std::any(123)
            )
    );


    auto result = store.find("test");
    if (result == store.end()){
        TEST_ASSERT_TRUE(false);
        return;
    } 

    TEST_ASSERT_EQUAL_STRING("test", result -> first.c_str());
    TEST_ASSERT_EQUAL_INT(123, std::any_cast<int>(result->second));
}

void setup()
{
    delay(2000);
    psramInit();

    UNITY_BEGIN();
    RUN_TEST(test_push_back);
    RUN_TEST(test_push_front);
    RUN_TEST(test_pop_front);
    RUN_TEST(test_pop_back);
    RUN_TEST(test_clear);
    RUN_TEST(test_at);
    RUN_TEST(test_erase);
    RUN_TEST(test_empty);
    RUN_TEST(test_size);
    RUN_TEST(test_ps_deque_psram_allocation);
    RUN_TEST(test_ps_deque_in_ps_deque);

    RUN_TEST(test_vector_operations);

    RUN_TEST(test_queue_functionality);

    RUN_TEST(test_make_shared);

    RUN_TEST(test_stringLength);
    RUN_TEST(test_stringConcatenation);
    RUN_TEST(test_string_in_queue);
    RUN_TEST(test_std_into_ps);
    RUN_TEST(test_ret_str_in_queue);

    RUN_TEST(test_unordered_map);

    RUN_TEST(test_std_any);

    RUN_TEST(test_std_any_in_unordered_map);
    UNITY_END();
}

void loop()
{
}