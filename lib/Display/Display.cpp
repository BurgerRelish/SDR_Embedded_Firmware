#include "Display.h"
#include <stdarg.h>
#include <esp_heap_caps.h>
#include "./frames/LoadingFrames.h"

Display::Display(uint8_t scl_pin, uint8_t sda_pin) : display(U8G2_R0, U8X8_PIN_NONE,/* clock=*/ scl_pin, /* data=*/ sda_pin /* reset=*/ ) {
    state = DISPLAY_SHOW_BLANK;
    update_required = false;
}

Display::~Display() {
    xSemaphoreTake(display_semaphore, portMAX_DELAY);
    vTaskDelete(display_task);
    vSemaphoreDelete(display_semaphore);
    display.setPowerSave(1);
}



void Display::begin(SummaryFrameData summary) {
    display_semaphore = xSemaphoreCreateBinary();
    summary_data = summary;
    display.setBusClock(DISPLAY_I2C_CLK_FREQUENCY_KHZ * 1000);
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

bool Display::startLoading() {
    if(xSemaphoreTake(display_semaphore, 250 / portTICK_PERIOD_MS) != pdTRUE) return false;  // Return if we cannot take display semaphore.
    state = DISPLAY_START_LOADING;
    update_required = true;
    xSemaphoreGive(display_semaphore);   
    return true; 
}

bool Display::finishLoading() {
    if(xSemaphoreTake(display_semaphore, 250 / portTICK_PERIOD_MS) != pdTRUE) return false;  // Return if we cannot take display semaphore.
    state = DISPLAY_FINISH_LOADING;
    update_required = true;
    xSemaphoreGive(display_semaphore);   
    return true; 
}

bool Display::showBlank() {
    if(xSemaphoreTake(display_semaphore, 250 / portTICK_PERIOD_MS) != pdTRUE) return false;  // Return if we cannot take display semaphore.
    state = DISPLAY_SHOW_BLANK;
    update_required = true;
    xSemaphoreGive(display_semaphore);   
    return true; 
}

bool Display::showLoadingFrame1() {
    if(xSemaphoreTake(display_semaphore, 250 / portTICK_PERIOD_MS) != pdTRUE) return false;  // Return if we cannot take display semaphore.
    state = DISPLAY_SHOW_LOADING_1;
    update_required = true;
    xSemaphoreGive(display_semaphore);
    return true;
}

bool Display::showLoadingFrame2() {
    if(xSemaphoreTake(display_semaphore, 250 / portTICK_PERIOD_MS) != pdTRUE) return false;  // Return if we cannot take display semaphore.
    state = DISPLAY_SHOW_LOADING_2;
    update_required = true;
    xSemaphoreGive(display_semaphore);
    return true;
}

bool Display::showLoadingFrame3() {
    if(xSemaphoreTake(display_semaphore, 250 / portTICK_PERIOD_MS) != pdTRUE) return false;  // Return if we cannot take display semaphore.
    state = DISPLAY_SHOW_LOADING_3;
    update_required = true;
    xSemaphoreGive(display_semaphore);
    return true;
}


bool Display::showLoadingComplete() {
    if(xSemaphoreTake(display_semaphore, 250 / portTICK_PERIOD_MS) != pdTRUE) return false;  // Return if we cannot take display semaphore.
    state = DISPLAY_SHOW_LOADING_COMPLETE;
    update_required = true;
    xSemaphoreGive(display_semaphore);
    return true;
}

bool Display::showSummary() {
    if(xSemaphoreTake(display_semaphore, 250 / portTICK_PERIOD_MS) != pdTRUE) return false;  // Return if we cannot take display semaphore.
    state = DISPLAY_SHOW_SUMMARY;
    update_required = true;
    xSemaphoreGive(display_semaphore);
    return true;
}

void displayTask(void* pvParameters) {
    Display* dsp = (Display*) pvParameters;
    uint32_t start_tm = 0;
    uint8_t counter = 0;

    while(1) {
        if ( ! dsp -> update_required ) { // Do not update the display for no reason, to save power.
            vTaskDelay((1000 / DISPLAY_UPDATE_HZ) / portTICK_PERIOD_MS);
            continue; 
        }

        xSemaphoreTake(dsp -> display_semaphore, portMAX_DELAY);
        start_tm = millis();
        
        dsp -> display.firstPage();
        do {
            switch ( dsp -> state ) {
                case Display::DISPLAY_START_LOADING:
                    if (counter == 0){
                        dsp -> drawLoadingFrame1();
                        break;
                    } else if (counter == 1) {
                        dsp -> drawLoadingFrame2();
                        break;
                    }

                    dsp -> drawLoadingFrame3();
                    break;
                case Display::DISPLAY_FINISH_LOADING: {
                    if (counter == 0) {
                        auto bitmap = dsp -> combineBitMaps(128, 64, 2, (uint8_t*) &epd_bitmap_Loading_Screen_1, (uint8_t*) &epd_bitmap_Loading_Screen_2);
                        dsp -> display.drawXBMP(0, 0, 128, 64, bitmap);
                        free(bitmap);           
                    } else if (counter == 1) {
                        auto bitmap = dsp -> combineBitMaps(128, 64, 2, (uint8_t*) &epd_bitmap_Loading_Screen_2, (uint8_t*) &epd_bitmap_Loading_Screen_3);
                        dsp -> display.drawXBMP(0, 0, 128, 64, bitmap);
                        free(bitmap); 
                    } else if (counter == 2) {
                        auto bitmap = dsp -> combineBitMaps(128, 64, 2, (uint8_t*) &epd_bitmap_Loading_Screen_1, (uint8_t*) &epd_bitmap_Loading_Screen_3);
                        dsp -> display.drawXBMP(0, 0, 128, 64, bitmap);
                        free(bitmap); 
                    }

                    auto bitmap = dsp -> combineBitMaps(128, 64, 3, (uint8_t*) &epd_bitmap_Loading_Screen_1, (uint8_t*) &epd_bitmap_Loading_Screen_2, (uint8_t*) &epd_bitmap_Loading_Screen_3);
                    dsp -> display.drawXBMP(0, 0, 128, 64, bitmap);
                    free(bitmap); 
                }

                break;
            }
        } while (dsp -> display.nextPage());

        switch( dsp -> state ) {
            case Display::DISPLAY_START_LOADING:
                if (counter >= 2) {
                    counter = 0;
                    break;
                }
                counter++;
                break;
            case Display::DISPLAY_FINISH_LOADING:
                if (counter < 3) {
                    counter = 3;
                    break;
                }
                counter = 5; // Stop Loading Screen Delay.
                break;
        }
        
        ESP_LOGV("", "Updated display.");

        xSemaphoreGive(dsp -> display_semaphore);

        uint32_t elapsed_tm = millis() - start_tm;
        if (counter <= 3) 
            vTaskDelay(500 / portTICK_PERIOD_MS); // Loading Screen Speed.
        else if (elapsed_tm < (1000 / DISPLAY_UPDATE_HZ)) // Do not delay the task if the frame rate is below desired.
            vTaskDelay(((1000 / DISPLAY_UPDATE_HZ) - elapsed_tm) / portTICK_PERIOD_MS); // Delay to acheive desired frame rate.
        else
            ESP_LOGE("Dsp", "Running behind target frame rate, elapsed: %d", elapsed_tm);

        if(millis() % 1000 < 100) ESP_LOGV("RTOS", "Stack: %d", uxTaskGetStackHighWaterMark(NULL));
    }

    vTaskDelete(NULL);

}

void Display::drawLoadingFrame1() {
    display.drawXBMP(0, 0, 128, 64, epd_bitmap_Loading_Screen_1);
}

void Display::drawLoadingFrame2() {
    display.drawXBMP(0, 0, 128, 64, epd_bitmap_Loading_Screen_2);

}

void Display::drawLoadingFrame3() {
    display.drawXBMP(0, 0, 128, 64, epd_bitmap_Loading_Screen_3);
}


void Display::drawLoadingFrameComplete() {
    display.firstPage();
    do {
        display.drawXBMP(0, 0, 128, 64, epd_bitmap_Loading_Screen_Complete);
    } while (display.nextPage());
}

void Display::drawSummaryFrame() {
    
    display.setFont(u8g2_font_4x6_tr);
    display.drawStr(1,54,"SDR Unit V1.0");
}


void Display::drawSignalStrengthIndicator() {
    if (*summary_data.connection_strength == 0) return; // No connection.

    if (*summary_data.connection_strength < -80) { // Not good

    }

    if (*summary_data.connection_strength < -70) { // Okay
        
    }

    if (*summary_data.connection_strength < -67) { // Good
        
    }

    if (*summary_data.connection_strength < -55) { // Very Good
        
    }
}

/**
 * @brief Combine multiple bitmaps into one.
 * 
 * @param w Width of bitmaps
 * @param h Height of bitmaps
 * @param num_bitmaps Number of bitmaps to be combined.
 * @param ... - uint8_t* pointers to bitmaps.
 * @return uint8_t* Pointer to combined bitmap. Free this memory when done!
 */
uint8_t* Display::combineBitMaps(uint8_t w, uint8_t h, uint8_t num_bitmaps, ...) {
    va_list args;
    va_start(args, num_bitmaps);

    // Validate input arguments
    if (num_bitmaps == 0) {
        return nullptr; // No bitmaps to combine
    }

    // Allocate memory for the combined bitmap
#ifdef BOARD_HAS_PSRAM
    uint8_t* ret = (uint8_t*) heap_caps_malloc(sizeof(uint8_t) * w * h, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
#else
    uint8_t* ret = (uint8_t*) malloc(sizeof(uint8_t) * w * h);
#endif

    // Check for memory allocation failure
    if (ret == nullptr) {
        return nullptr; // Allocation failed
    }

    // Initialize the combined bitmap with the first bitmap
    uint8_t* bmp_0 = va_arg(args, uint8_t*);
    memcpy(ret, bmp_0, sizeof(uint8_t) * w * h);

    // Combine the remaining bitmaps
    for (int i = 1; i < num_bitmaps; i++) {
        uint8_t* bitmap = va_arg(args, uint8_t*);
        for (uint32_t j = 0; j < (w * h); j++) {
            *(ret + j) |= *(bitmap + j);
        }
    }

    va_end(args);
    
    return ret;
}