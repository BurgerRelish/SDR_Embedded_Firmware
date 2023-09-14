#pragma once

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#define DISPLAY_UPDATE_HZ 30
#define OLED_PANEL U8G2_SSD1306_128X64_NONAME_F_SW_I2C

void displayTask(void* pvParameters);

struct SummaryFrameData {
    double* total_apparent_power;
    double* mean_voltage;
    double* mean_frequency;
    bool* power_status;
};

class Display {
    public:
    Display(uint8_t scl_pin, uint8_t sda_pin);
    ~Display();

    void begin(SummaryFrameData summary);
    
    bool showBlank();
    bool showLogoFrame1();
    bool showLogoFrame2();
    bool showSummary();

    private:
    friend void displayTask(void* pvParameters);
    enum DisplayState {
        DISPLAY_SHOW_LOGO_1,
        DISPLAY_SHOW_LOGO_2,
        DISPLAY_SHOW_SUMMARY,
        DISPLAY_SHOW_BLANK
    } state;

    OLED_PANEL display;
    bool update_required;

    TaskHandle_t display_task;
    SemaphoreHandle_t display_semaphore;

    SummaryFrameData summary_data;

    void drawLogoFrame1();
    void drawLogoFrame2();
    void drawSummaryFrame();
};

#endif