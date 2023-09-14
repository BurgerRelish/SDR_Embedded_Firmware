#include "Display.h"

Display::Display(uint8_t scl_pin, uint8_t sda_pin) : display(U8G2_R0, /* clock=*/ scl_pin, /* data=*/ sda_pin, /* reset=*/ U8X8_PIN_NONE) {
    state = DISPLAY_SHOW_BLANK;
    update_required = false;
}

Display::~Display() {
    xSemaphoreTake(display_semaphore, portMAX_DELAY);
    vTaskDelete(display_task);
    vSemaphoreDelete(display_semaphore);
    display.clearBuffer();
    display.sendBuffer();
}

void Display::begin(SummaryFrameData summary) {
    display_semaphore = xSemaphoreCreateBinary();
    summary_data = summary;
    display.setBusClock(400000);
    display.begin();

    xTaskCreate(
        displayTask,
        "Display",
        5000,
        this,
        0,
        &display_task
    );

    xSemaphoreGive(display_semaphore);
}

bool Display::showBlank() {
    if(xSemaphoreTake(display_semaphore, 50 / portTICK_PERIOD_MS) != pdTRUE) return false;  // Return if we cannot take display semaphore.
    state = DISPLAY_SHOW_BLANK;
    update_required = true;
    xSemaphoreGive(display_semaphore);   
    return true; 
}

bool Display::showLogoFrame1() {
    if(xSemaphoreTake(display_semaphore, 50 / portTICK_PERIOD_MS) != pdTRUE) return false;  // Return if we cannot take display semaphore.
    state = DISPLAY_SHOW_LOGO_1;
    update_required = true;
    xSemaphoreGive(display_semaphore);
    return true;
}

bool Display::showLogoFrame2() {
    if(xSemaphoreTake(display_semaphore, 50 / portTICK_PERIOD_MS) != pdTRUE) return false;  // Return if we cannot take display semaphore.
    state = DISPLAY_SHOW_LOGO_2;
    update_required = true;
    xSemaphoreGive(display_semaphore);
    return true;
}

bool Display::showSummary() {
    if(xSemaphoreTake(display_semaphore, 50 / portTICK_PERIOD_MS) != pdTRUE) return false;  // Return if we cannot take display semaphore.
    state = DISPLAY_SHOW_SUMMARY;
    update_required = true;
    xSemaphoreGive(display_semaphore);
    return true;
}

void displayTask(void* pvParameters) {
    Display* dsp = (Display*) pvParameters;
    uint32_t start_tm = 0;

    while(1) {
        
        if ( !dsp -> update_required ) continue; // Do not update the display for no reason to save power.
        if(xSemaphoreTake(dsp -> display_semaphore, 50 / portTICK_PERIOD_MS) != pdTRUE) continue;  // Try again if we cannot take display semaphore.
        start_tm = millis();
        
        dsp -> display.clearBuffer();

        switch ( dsp -> state ) {
            case Display::DISPLAY_SHOW_BLANK:
                dsp -> display.clear();
                dsp -> update_required = false;
                break;
            case Display::DISPLAY_SHOW_LOGO_1:
                dsp -> drawLogoFrame1();
                dsp -> update_required = false;
                break;
            case Display::DISPLAY_SHOW_LOGO_2:
                dsp -> drawLogoFrame2();
                dsp -> update_required = false;
                break;
            case Display::DISPLAY_SHOW_SUMMARY:
                dsp -> drawSummaryFrame();
                break;

                
            default:
                break;
        }

        dsp -> display.sendBuffer();

        xSemaphoreGive(dsp -> display_semaphore);
        uint32_t elapsed_tm = millis() - start_tm;
        if (elapsed_tm < (1000 / DISPLAY_UPDATE_HZ)) // Do not delay the task if the frame rate is below desired.
            vTaskDelay(((1000 / DISPLAY_UPDATE_HZ) - elapsed_tm) / portTICK_PERIOD_MS); // Delay to acheive desired frame rate.
        else
            ESP_LOGE("Dsp", "Running behind target frame rate.");

        if(millis() % 1000 < 100) ESP_LOGI("RTOS", "Stack: %d", uxTaskGetStackHighWaterMark(NULL));
    }

    vTaskDelete(NULL);

}

void Display::drawLogoFrame1() {
    display.setFontMode(1);
    display.setFontDirection(0);
    display.setFont(u8g2_font_inb24_mf);
    display.drawStr(0, 30, "U");
    
    display.setFontDirection(1);
    display.setFont(u8g2_font_inb30_mn);
    display.drawStr(21,8,"8");
        
    display.setFontDirection(0);
    display.setFont(u8g2_font_inb24_mf);
    display.drawStr(51,30,"g");
    display.drawStr(67,30,"\xb2");
    
    display.drawHLine(2, 35, 47);
    display.drawHLine(3, 36, 47);
    display.drawVLine(45, 32, 12);
    display.drawVLine(46, 33, 12);
}

void Display::drawLogoFrame2() {
    drawLogoFrame1();
}

void Display::drawSummaryFrame() {
    display.setFont(u8g2_font_4x6_tr);
    display.drawStr(1,54,"SDR Unit V1.0");
}
