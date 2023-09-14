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

    while(1) {
        if ( ! dsp -> update_required ) { // Do not update the display for no reason, to save power.
            vTaskDelay((1000 / DISPLAY_UPDATE_HZ) / portTICK_PERIOD_MS);
            continue; 
        }

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
                dsp -> display.firstPage();
                do {
                    dsp -> drawSplashFrame();
                } while (dsp -> display.nextPage());
                break;

            case Display::DISPLAY_SHOW_BLANK:
                dsp -> display.clear();
                dsp -> update_required = false;
                break;

            case Display::DISPLAY_SHOW_SUMMARY:
                dsp -> display.firstPage();
                do {
                    dsp -> drawSummaryFrame();
                } while (dsp -> display.nextPage());
                break;
            
            default:
            break;
        }

        /* Animation logic state machine. */
        switch( dsp -> state ) {
            case Display::DISPLAY_START_LOADING:
                if (animation_state >= 2) {
                    animation_state = 0;
                    break;
                }
                animation_state++;
                break;
            case Display::DISPLAY_FINISH_LOADING:
                if (animation_state < 3) {
                    animation_state = 3;
                    break;
                }
                dsp -> state = Display::DISPLAY_SHOW_SPLASH;
                break;

            case Display::DISPLAY_SHOW_SPLASH:
                animation_state = 5; // Stop Loading Screen Delay.
                dsp -> update_required = false;
                break;
        }
        
        ESP_LOGV("", "Updated display.");

        xSemaphoreGive(dsp -> display_semaphore);

        uint32_t elapsed_tm = millis() - start_tm;
        if (animation_state <= 3) 
            vTaskDelay(500 / portTICK_PERIOD_MS); // Loading Screen Speed.
        else if (elapsed_tm < (1000 / DISPLAY_UPDATE_HZ)) // Do not delay the task if the frame rate is below desired.
            vTaskDelay(((1000 / DISPLAY_UPDATE_HZ) - elapsed_tm) / portTICK_PERIOD_MS); // Delay to acheive desired frame rate.
        else
            ESP_LOGE("Dsp", "Running behind target frame rate, elapsed: %d", elapsed_tm);

        if(millis() % 1000 < 100) ESP_LOGV("RTOS", "Stack: %d", uxTaskGetStackHighWaterMark(NULL));
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
    display.setFont(u8g2_font_haxrcorp4089_tr);
    display.drawStr(1, 28, "0                      %NMD                       1");
    display.drawBox(3, 3, 81, 7);
    display.setFont(u8g2_font_profont22_tr);
    display.drawStr(2, 44, "13284VAh");
    display.setFont(u8g2_font_helvB08_tr);
    display.drawStr(100, 44, "1.234");
    display.setFont(u8g2_font_haxrcorp4089_tr);
    display.drawStr(3, 54, "230.25V");
    display.setFont(u8g2_font_haxrcorp4089_tr);
    display.drawStr(3, 63, "49.93Hz");
    display.setFont(u8g2_font_4x6_tr);
    display.drawStr(51, 63, "21:49:15 14/09/2023");
    display.drawBox(54, 53, 2, 3);
    display.drawBox(58, 51, 2, 5);
    display.drawBox(62, 49, 2, 7);
    display.drawBox(66, 47, 2, 9);
    display.setFont(u8g2_font_haxrcorp4089_tr);
    display.drawStr(95, 56, "99/99");
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

void Display::drawSplashFrame() {
    display.setBitmapMode(1);
    display.setFont(u8g2_font_profont22_tr);
    display.drawStr(17, 15, "S");
    display.setFont(u8g2_font_profont22_tr);
    display.drawStr(39, 49, "R");
    display.setFont(u8g2_font_4x6_tr);
    display.drawStr(1, 63, "Version: 0000/0000");
    display.setFont(u8g2_font_profont22_tr);
    display.drawStr(28, 32, "D");
    display.setFont(u8g2_font_helvB08_tr);
    display.drawStr(31, 15, "MART");
    display.setFont(u8g2_font_helvB08_tr);
    display.drawStr(42, 32, "EMAND");
    display.setFont(u8g2_font_helvB08_tr);
    display.drawStr(53, 49, "ESPONSE");
}