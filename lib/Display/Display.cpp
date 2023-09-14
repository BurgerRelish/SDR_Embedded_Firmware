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

void Display::begin(SummaryFrameData* summary) {
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
    state = DISPLAY_SHOW_SPLASH;
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
    uint8_t animation_state = 0;
    uint32_t delay_tm = 0;

    while(1) {
        if ( ! dsp -> update_required ) { // Do not update the display for no reason, to save power.
            vTaskDelay((1000 / DISPLAY_UPDATE_CHECK_FREQUENCY_HZ) / portTICK_PERIOD_MS);
            continue; 
        }

        vTaskDelay(delay_tm / portTICK_PERIOD_MS);

        xSemaphoreTake(dsp -> display_semaphore, portMAX_DELAY);
        start_tm = millis();
        
        dsp -> display.firstPage();

        /* Display Frame Rendering State Machine. */
        switch ( dsp -> state ) {
            case Display::DISPLAY_START_LOADING: {
                uint8_t* bitmap = nullptr;

                switch( animation_state ) {
                    case 0:
                        bitmap = (uint8_t*) &epd_bitmap_Loading_Screen_1;
                        break;
                    case 1:
                        bitmap = (uint8_t*) &epd_bitmap_Loading_Screen_2;
                        break;
                    default:
                        bitmap = (uint8_t*) &epd_bitmap_Loading_Screen_3;
                        break;
                }

                do {
                    dsp -> display.drawXBMP(0, 0, 128, 64, bitmap);
                } while(dsp -> display.nextPage());

                break;
            }

            case Display::DISPLAY_FINISH_LOADING: {
                uint8_t* bitmap = nullptr;

                switch( animation_state ) {
                    case 0:
                        bitmap = dsp -> combineBitMaps(128, 64, 2, (uint8_t*) &epd_bitmap_Loading_Screen_1, (uint8_t*) &epd_bitmap_Loading_Screen_3);
                        break;
                    case 1:
                        bitmap = dsp -> combineBitMaps(128, 64, 2, (uint8_t*) &epd_bitmap_Loading_Screen_1, (uint8_t*) &epd_bitmap_Loading_Screen_2);
                        break;
                    case 2:
                        bitmap = dsp -> combineBitMaps(128, 64, 2, (uint8_t*) &epd_bitmap_Loading_Screen_2, (uint8_t*) &epd_bitmap_Loading_Screen_3);
                        break;
                    default:
                        bitmap = dsp -> combineBitMaps(128, 64, 3, (uint8_t*) &epd_bitmap_Loading_Screen_1, (uint8_t*) &epd_bitmap_Loading_Screen_2, (uint8_t*) &epd_bitmap_Loading_Screen_3);
                        break;
                }

                do {
                    dsp -> display.drawXBMP(0, 0, 128, 64, bitmap);
                } while(dsp -> display.nextPage());

                free(bitmap);
                break;
            }

            case Display::DISPLAY_SHOW_SPLASH:
                do {
                    dsp -> drawSplashFrame();
                } while (dsp -> display.nextPage());
                break;

            case Display::DISPLAY_SHOW_BLANK:
                dsp -> display.clear();
                dsp -> update_required = false;
                break;

            case Display::DISPLAY_SHOW_SUMMARY:
                do {
                    dsp -> drawSummaryFrame();
                } while (dsp -> display.nextPage());
                break;
            
            default:
            break;
        }

        uint32_t elapsed_tm = millis() - start_tm;


        /* Animation logic state machine. */
        switch( dsp -> state ) {
            case Display::DISPLAY_START_LOADING:
                if (elapsed_tm < LOADING_SCROLL_SPEED) { // Wait till next frame.
                    delay_tm = LOADING_SCROLL_SPEED - elapsed_tm;
                } else delay_tm = 0;

                if (animation_state >= 2) { // Loop around.
                    animation_state = 0;
                    break;
                }

                animation_state++;
                break;
            case Display::DISPLAY_FINISH_LOADING:
                if (elapsed_tm < LOADING_SCROLL_SPEED) {
                    delay_tm = LOADING_SCROLL_SPEED - elapsed_tm;
                } else delay_tm = 0;
                
                if (animation_state < 3) {
                    animation_state = 3;
                    break;
                }

                dsp -> update_required = false;
                break;

            case Display::DISPLAY_SHOW_SPLASH:
                dsp -> state = Display::DISPLAY_START_LOADING;
                delay_tm = SPLASH_DURATION_MS - elapsed_tm;
                break;

            case Display::DISPLAY_SHOW_SUMMARY:
                delay_tm = SUMMARY_POLL_RATE_MS - elapsed_tm;
        }
        
        ESP_LOGI("DISPLAY", "Updated display. Took: %u", elapsed_tm);
        xSemaphoreGive(dsp -> display_semaphore);
        if(millis() % 1000 < 100) ESP_LOGI("DISPLAY", "Stack: %d", uxTaskGetStackHighWaterMark(NULL));
    }

    vTaskDelete(NULL);

}

void Display::drawSummaryFrame() {
    display.setBitmapMode(1);
    display.drawFrame(1, 1, 126, 11);
    display.drawLine(1, 14, 1, 18);
    display.drawLine(126, 14, 126, 18);
    display.drawLine(32, 14, 32, 16);
    display.drawLine(64, 14, 64, 16);
    display.drawLine(98, 14, 98, 16);
    
    uint8_t nmd_bar_width = map(summary_data -> total_apparent_power, 0, summary_data -> nmd, 0, 125);
    display.drawBox(3, 3, nmd_bar_width, 7);
    display.setFont(u8g2_font_profont22_tr);

    if ((summary_data->total_apparent_power) >= 10000) {
        drawStrf(2, 44, "%.0fVA", (summary_data->total_apparent_power));
    } else if ((summary_data->total_apparent_power) >= 1000) {
        drawStrf(2, 44, "%.1fVA", (summary_data->total_apparent_power));
    } else {
        drawStrf(2, 44, "%.2fVA", (summary_data->total_apparent_power));
    }
    
    display.setFont(u8g2_font_helvB08_tr);
    drawStrf(100, 44, "%1.3f", (summary_data->mean_power_factor));
    display.setFont(u8g2_font_haxrcorp4089_tr);
    drawStrf(3, 54, "%3.2fV", (summary_data->mean_voltage));
    display.drawStr(1, 28, "0                      %NMD                       1");
    drawStrf(95, 56, "%d/%d", (summary_data->on_modules), (summary_data->total_modules));
    drawStrf(3, 63, "%2.2fHz", (summary_data->mean_frequency));
    display.setFont(u8g2_font_4x6_tr);
    display.drawStr(51, 63, "21:49:15 14/09/2023");
    display.setFont(u8g2_font_haxrcorp4089_tr);
    

    drawSignalStrengthIndicator();
}



void Display::drawSignalStrengthIndicator() {
    if (summary_data->connection_strength == 0) return; // No connection.
        
    if (summary_data->connection_strength > -80) { // Not good
        display.drawBox(54, 53, 2, 3);
    }

    if (summary_data->connection_strength > -70) { // Okay
        display.drawBox(58, 51, 2, 5);
    }

    if (summary_data->connection_strength > -67) { // Good
        display.drawBox(62, 49, 2, 7);
    }

    if (summary_data->connection_strength > -55) { // Very Good
        display.drawBox(66, 47, 2, 9);
    }
}

void Display::drawStrf(uint8_t x, uint8_t y, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char* str = (char*) malloc(sizeof(char) * 128);
    vsnprintf(str, 128, format, args);
    display.drawStr(x, y, str);

    free(str);
    va_end(args);
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

void Display::drawSplashFrame() {
    display.setBitmapMode(1);

    display.setFont(u8g2_font_profont22_tr);
    display.drawStr(17, 15, "S");
    display.drawStr(28, 32, "D");
    display.drawStr(39, 49, "R");

    display.setFont(u8g2_font_helvB08_tr);
    display.drawStr(29, 15, "MART");
    display.drawStr(40, 32, "EMAND");
    display.drawStr(51, 49, "ESPONSE");

    display.setFont(u8g2_font_4x6_tr);
    display.drawStr(1, 63, "Version: 0000/0000");
}