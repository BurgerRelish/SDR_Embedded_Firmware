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
#define SPLASH_DURATION_MS 1000
#define LOADING_CPLT_DURATION_MS 350
#define SUMMARY_POLL_RATE_MS 1000

using OLED_PANEL = U8G2_SSD1306_128X64_NONAME_F_HW_I2C; // SSD1036, 128x64, Full Frame buffer, Hardware I2C

void displayTask(void* pvParameters);

/**
 * @brief Data to display on the summary screen.
 * 
 *  Structure:
    @param double total_apparent_power
    @param double mean_power_factor
    @param double mean_voltage
    @param double mean_frequency
    @param double nmd
    @param bool power_status
    @param int8_t connection_strength
    @param uint16_t total_modules
    @param uint16_t on_modules
 *
 */
struct SummaryFrameData {
    double total_apparent_power;
    double mean_power_factor;
    double mean_voltage;
    double mean_frequency;
    double current;
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

    enum DisplayState {
        DISPLAY_LOADING_WHEEL,
        DISPLAY_LOADED_ANIMATION,
        DISPLAY_SHOW_SPLASH,
        DISPLAY_SHOW_SUMMARY,
        DISPLAY_SHOW_BLANK
    };

    void begin(SummaryFrameData* summary);
    
    bool finishLoading();

    bool showBlank();
    bool showSummary();

    bool pause();
    void resume();

    private:
    friend void displayTask(void* pvParameters);
    DisplayState state;

    OLED_PANEL oled;

    QueueHandle_t display_queue;
    TaskHandle_t display_task;
    SemaphoreHandle_t display_semaphore;
    SummaryFrameData* summary_data;

    struct DisplayQueuePacket {
        DisplayState state;
    };

    const char* version;

    void drawSplashFrame();
    void drawSummaryFrame();
    void drawSignalStrengthIndicator();
    void drawStrf(uint8_t x, uint8_t y, const char* format, ...);

    void combineBitMaps(uint8_t* bitmap, size_t size, uint8_t num_bitmaps, ...);
};

#endif