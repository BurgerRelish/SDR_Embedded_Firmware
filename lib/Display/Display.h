#pragma once

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>
#include <ps_stl.h>

#define DISPLAY_UPDATE_CHECK_FREQUENCY_HZ 10
#define DISPLAY_I2C_CLK_FREQUENCY_KHZ 400
#define LOADING_SCROLL_SPEED 350 // Only values above > 130 ms due to I2C speed.
#define SPLASH_DURATION_MS 500
#define LOADING_CPLT_DURATION_MS 350
#define SUMMARY_POLL_RATE_MS 1000

using OLED_PANEL = U8G2_SSD1306_128X64_NONAME_F_HW_I2C; // SSD1036, 128x64, Full Frame buffer, Hardware I2C

void displayTask(void* pvParameters);

struct SummaryFrameData {
    double total_apparent_power;
    double mean_power_factor;
    double mean_voltage;
    double mean_frequency;
    double nmd;
    bool power_status;
    int8_t connection_strength;
    uint16_t total_modules;
    uint16_t on_modules;
};

class Display {
    public:
    Display(uint8_t scl_pin, uint8_t sda_pin, const char* version);
    ~Display();

    void begin(SummaryFrameData* summary);
    
    bool startLoading();
    bool finishLoading();

    bool showBlank();
    bool showSummary();

    bool pause();
    void resume();

    private:
    friend void displayTask(void* pvParameters);
    enum DisplayState {
        DISPLAY_START_LOADING,
        DISPLAY_FINISH_LOADING,
        DISPLAY_SHOW_SPLASH,
        DISPLAY_SHOW_SUMMARY,
        DISPLAY_SHOW_BLANK
    } state;

    OLED_PANEL display;
    bool update_required;
    
    const char* version;

    TaskHandle_t display_task;
    SemaphoreHandle_t display_semaphore;
    SummaryFrameData* summary_data;

    void drawSplashFrame();
    void drawSummaryFrame();
    void drawSignalStrengthIndicator();
    void drawStrf(uint8_t x, uint8_t y, const char* format, ...);

    void combineBitMaps(uint8_t* bitmap, size_t size, uint8_t num_bitmaps, ...);
};

#endif