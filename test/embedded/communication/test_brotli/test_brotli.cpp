#include <Arduino.h>
#include <unity.h>
#include <string>
#include "ps_string.h"
#include "MessageSerializer.h"


void test_compression() {
    MessageSerializer compressor;
    ps::string message = "{ \"timestamp\": 1685632656, \"RD\": [ {\"mid\": \"1234598127\",\"avgV\": 228.08,\"maxV\": 234.5,\"minV\": 226.49,\"stdV\": 1.2312,\"avgF\": 49.431,\"maxF\": 50.670,\"minF\": 48.897,\"stdF\": 0.2131,\"avgSP\": 704.00,\"maxSP\": 765.23,\"minSP\": 671.58,\"stdSP\": 43.764,\"avgPF\": 385.75,\"maxPF\": 586.00,\"minPF\": 125.50,\"stdPF\": 34.239,\"KWH\": 0.9082,\"state\": true,\"sw_time\": 1685732656}]}";

    ps::string result = compressor.compressString(message);
    TEST_ASSERT_NOT_EQUAL(0, result.length());
    TEST_ASSERT_EQUAL_STRING("G2IBIJwHto3s7eKlwRaCtVWqzkK8eLIcO18JPXoITrNuTvtNc060PFFT8jz5oZZSFYtOgYhY3qrfJ9MoQrR9CPEmCpoRjKRfqBHkjlzqtXdEpvbr3MpvFL3VVlc/zi+gv/d0QyfkrLuhOeodBUkL7nPqj+NEZfD11MhL3McoIIemhSYv0iGkInpYQpOLvSN0TOHoWcoMEyRaNF2QBagnUCtAaUynvNB4C1OAahskYIQKJSickO9KNS6UL1dwtuH97/26ndVJnfm1Dw==",
                            result.c_str());

}

void test_decompression() {
    MessageSerializer decompressor;
    ps::string message = "G2IBIJwHto3s7eKlwRaCtVWqzkK8eLIcO18JPXoITrNuTvtNc060PFFT8jz5oZZSFYtOgYhY3qrfJ9MoQrR9CPEmCpoRjKRfqBHkjlzqtXdEpvbr3MpvFL3VVlc/zi+gv/d0QyfkrLuhOeodBUkL7nPqj+NEZfD11MhL3McoIIemhSYv0iGkInpYQpOLvSN0TOHoWcoMEyRaNF2QBagnUCtAaUynvNB4C1OAahskYIQKJSickO9KNS6UL1dwtuH97/26ndVJnfm1Dw==";

    ps::string result = decompressor.decompressString(message);
    TEST_ASSERT_NOT_EQUAL(0, result.length());
    TEST_ASSERT_EQUAL_STRING("{ \"timestamp\": 1685632656, \"RD\": [ {\"mid\": \"1234598127\",\"avgV\": 228.08,\"maxV\": 234.5,\"minV\": 226.49,\"stdV\": 1.2312,\"avgF\": 49.431,\"maxF\": 50.670,\"minF\": 48.897,\"stdF\": 0.2131,\"avgSP\": 704.00,\"maxSP\": 765.23,\"minSP\": 671.58,\"stdSP\": 43.764,\"avgPF\": 385.75,\"maxPF\": 586.00,\"minPF\": 125.50,\"stdPF\": 34.239,\"KWH\": 0.9082,\"state\": true,\"sw_time\": 1685732656}]}",
                            result.c_str());

}

void setup() {
    psramInit();
    delay(2000);
    UNITY_BEGIN();
    RUN_TEST(test_compression);
    RUN_TEST(test_decompression);
    UNITY_END();
}

void loop() {

}