#pragma once

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include <Wire.h>
#include <U8g2lib.h>

#define DISPLAY_UPDATE_HZ 5
#define OLED_PANEL U8G2_SSD1306_128X64_NONAME_2_HW_I2C // SSD1036, 128x64, 2 Frame buffers, Hardware I2C
#define DISPLAY_I2C_CLK_FREQUENCY_KHZ 1000

void displayTask(void* pvParameters);

struct SummaryFrameData {
    double* total_apparent_power;
    double* mean_voltage;
    double* mean_frequency;
    bool* power_status;
    int8_t* connection_strength;
};

class Display {
    public:
    Display(uint8_t scl_pin, uint8_t sda_pin);
    ~Display();

    void begin(SummaryFrameData summary);
    
    bool startLoading();
    bool finishLoading();

    bool showBlank();
    bool showLoadingFrame1();
    bool showLoadingFrame2();
    bool showLoadingFrame3();
    bool showLoadingComplete();
    bool showSummary();

    private:
    friend void displayTask(void* pvParameters);
    enum DisplayState {
        DISPLAY_START_LOADING,
        DISPLAY_FINISH_LOADING,
        DISPLAY_SHOW_LOADING_1,
        DISPLAY_SHOW_LOADING_2,
        DISPLAY_SHOW_LOADING_3,
        DISPLAY_SHOW_LOADING_COMPLETE,
        DISPLAY_SHOW_SUMMARY,
        DISPLAY_SHOW_BLANK
    } state;

    OLED_PANEL display;
    bool update_required;
    

    TaskHandle_t display_task;
    SemaphoreHandle_t display_semaphore;

    SummaryFrameData summary_data;

    void drawLoadingFrame1();
    void drawLoadingFrame2();
    void drawLoadingFrame3();
    void drawLoadingFrameComplete();
    void drawSummaryFrame();
    void drawSignalStrengthIndicator();

    uint8_t* combineBitMaps(uint8_t w, uint8_t h, uint8_t num_bitmaps, ...);
};

#endif