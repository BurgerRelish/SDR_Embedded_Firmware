#include <Arduino.h>
#include <unity.h>
#include <string>
#include "MessageSerializer.h"


void test_compression() {
    MessageSerializer compressor;
    std::string message = "{ \"timestamp\": 1685632656, \"RD\": [ {\"mid\": \"1234598127\",\"avgV\": 228.08,\"maxV\": 234.5,\"minV\": 226.49,\"stdV\": 1.2312,\"avgF\": 49.431,\"maxF\": 50.670,\"minF\": 48.897,\"stdF\": 0.2131,\"avgSP\": 704.00,\"maxSP\": 765.23,\"minSP\": 671.58,\"stdSP\": 43.764,\"avgPF\": 385.75,\"maxPF\": 586.00,\"minPF\": 125.50,\"stdPF\": 34.239,\"KWH\": 0.9082,\"state\": true,\"sw_time\": 1685732656}]}";

    std::string result = compressor.compressString(&message);
    ESP_LOGD("TESTBROTLI", "Compressed: \'%s\'", result.c_str());
    TEST_ASSERT_NOT_EQUAL(0, result.length());
    TEST_ASSERT_EQUAL_STRING("C7EAAICqqqrq/3QVkLOCgKqIiqiongMCAiAuDhEAHoeAgAAFcAMHPahDQJiGhwM4OKR4XQaMrQyLw+EwABjAJ3xU3DCi4YP/3gHXmNu++vzBBslcTdjUAuDbEzb4hDvOccIGmFiyVk9cMGC/no/YgNkpesDZb0dswJJJA85xEQU2yjXgvk6EJWJJPFd4xga5UpZ0VOQayUoMe4osO3ktexXdSJwG5H49vx+wQYmZ4oLpN1vPlFgKanMridTD87RZFiqWG9qHJi6uVDQ0aDvq1lwzLiZLrKSxobYkmVhqwNePlx/p1uh8gw99bdhg/f5tAff/7zWmkZO5FmFTe3w9Bg==",
                            result.c_str());
                            
}

void setup() {
    psramInit();
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_compression);
    UNITY_END();
}

void loop() {

}