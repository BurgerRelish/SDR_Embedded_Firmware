#include <Arduino.h>
#include <unity.h>
#include <queue>
#include "ps_deque.h"
#include <esp32-hal-psram.h>

void test_push_back()
{
    ps_deque<int> deque;
    deque.push_back(1);
    deque.push_back(2);
    deque.push_back(3);

    std::deque<int> std_queue;

    deque <<= std_queue;

    TEST_ASSERT_EQUAL(3, deque.size());
    TEST_ASSERT_EQUAL(1, deque.front());
    TEST_ASSERT_EQUAL(3, deque.back());
}

void test_push_front()
{
    ps_deque<int> deque;
    deque.push_front(3);
    deque.push_front(2);
    deque.push_front(1);

    TEST_ASSERT_EQUAL(3, deque.size());
    TEST_ASSERT_EQUAL(1, deque.front());
    TEST_ASSERT_EQUAL(3, deque.back());
}

void test_pop_front()
{
    ps_deque<int> deque;
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
    ps_deque<int> deque;
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
    ps_deque<int> deque;
    deque.push_back(1);
    deque.push_back(2);
    deque.push_back(3);

    deque.clear();

    TEST_ASSERT_EQUAL(0, deque.size());
    TEST_ASSERT_TRUE(deque.empty());
}

void test_at()
{
    ps_deque<int> deque;
    deque.push_back(1);
    deque.push_back(2);
    deque.push_back(3);

    TEST_ASSERT_EQUAL(2, deque.at(1));
    TEST_ASSERT_EQUAL(3, deque.at(2));
}

void test_erase()
{
    ps_deque<int> deque;
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
    ps_deque<int> deque;
    TEST_ASSERT_TRUE(deque.empty());

    deque.push_back(1);
    TEST_ASSERT_FALSE(deque.empty());

    deque.pop_back();
    TEST_ASSERT_TRUE(deque.empty());
}

void test_size()
{
    ps_deque<int> deque;
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
  // Create a ps_deque of integers
  ps_deque<int> myDeque;

  // Add some elements to the deque
  myDeque.push_back(1);
  myDeque.push_back(2);
  myDeque.push_back(3);

  // Check if the memory is allocated in PSRAM
  for (const auto& num : myDeque) {
    TEST_ASSERT_TRUE_MESSAGE(isPsramAllocated((void*)&num), "Element in ps_deque is not allocated in PSRAM");
  }
}

void test_ps_deque_in_ps_deque() {
    ps_deque<int> deque;
    ps_deque<ps_deque<int>> nested_deque;

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
    UNITY_END();
}

void loop()
{
}