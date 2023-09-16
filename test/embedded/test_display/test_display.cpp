#include <Arduino.h>
#include <unity.h>
#include <WiFi.h>
#include <FS.h>
#include "Display.h"

Display dsp(1, 2);

SummaryFrameData summary_data;

double mean_sp = 420.69;
double mean_pf = 1.529;
double mean_voltage = 229.45;
int8_t rssi = -68;
double mean_frequency = 49.78;
uint16_t on_modules = 1;
uint16_t total_modules = 5;
bool power_status = true;

void test_display() {

    dsp.begin(&summary_data);
    dsp.startLoading();
    delay(5000);
    dsp.finishLoading();
    delay(500);

    summary_data.connection_strength = rssi;
    summary_data.mean_frequency = mean_frequency;
    summary_data.mean_voltage = mean_voltage;
    summary_data.total_apparent_power =  mean_sp;
    summary_data.on_modules = on_modules;
    summary_data.total_modules = total_modules;
    summary_data.mean_power_factor = mean_pf;
    summary_data.power_status = power_status;
    summary_data.nmd = 13284;

    dsp.showSummary();

    delay(1000);
    summary_data.total_apparent_power = 6500.13;
    
    TEST_ASSERT_TRUE(true);
}

void setup() {
    //delay(1000);
    UNITY_BEGIN();
    RUN_TEST(test_display);
    UNITY_END();
}

void loop() {

}
