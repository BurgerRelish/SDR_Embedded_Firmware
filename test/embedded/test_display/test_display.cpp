#include <Arduino.h>
#include <unity.h>
#include "Display.h"

Display dsp(1, 2);

SummaryFrameData summary_data = {
    nullptr,
    nullptr,
    nullptr,
    nullptr
};

void test_display() {
    dsp.begin(summary_data);

    dsp.startLoading();
    delay(5000);
    dsp.finishLoading();
    delay(2500);
    TEST_ASSERT_TRUE(true);
}

void setup() {
    delay(1000);
    UNITY_BEGIN();
    RUN_TEST(test_display);
    UNITY_END();
}

void loop() {

}
