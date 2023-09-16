#include "Display.h"
#include <stdarg.h>
#include <esp_heap_caps.h>
#include "./frames/LoadingFrames.h"
#include <time.h>


Display::Display(uint8_t scl_pin, uint8_t sda_pin, const char* version) : oled(U8G2_R0, U8X8_PIN_NONE, scl_pin, sda_pin), version(version) {
    state = DISPLAY_SHOW_SPLASH;
}

Display::~Display() {
    xSemaphoreTake(display_semaphore, portMAX_DELAY);
    vTaskDelete(display_task);
    vSemaphoreDelete(display_semaphore);
    oled.setPowerSave(1);
}

void Display::begin(SummaryFrameData* summary) {
    display_semaphore = xSemaphoreCreateMutex();
    display_queue = xQueueCreate(10, sizeof(DisplayQueuePacket));

    summary_data = summary;
    oled.setBusClock(DISPLAY_I2C_CLK_FREQUENCY_KHZ * 1000);
    oled.begin();
    ESP_LOGI("Display", "begun.");
    if (xTaskCreate(
        displayTask,
        "Display",
        5 * 1024,
        this,
        5,
        &display_task
    ) != pdTRUE) {
        ESP_LOGE("RTOS", "Failed to start display task.");
    }

    xSemaphoreGive(display_semaphore);
}

bool Display::finishLoading() {
    DisplayQueuePacket packet;
    packet.state = DISPLAY_LOADED_ANIMATION;
    ESP_LOGI("Display", "Loading finished.");
    return (xQueueSendToBack(display_queue, &packet, 50 / portTICK_PERIOD_MS) == pdTRUE);
}

bool Display::showBlank() {
    DisplayQueuePacket packet;
    packet.state = DISPLAY_SHOW_BLANK;
    ESP_LOGI("Display", "Showing Blank.");
    return (xQueueSendToBack(display_queue, &packet, 50 / portTICK_PERIOD_MS) == pdTRUE);
}

bool Display::showSummary() {
    DisplayQueuePacket packet;
    packet.state = DISPLAY_SHOW_SUMMARY;
    ESP_LOGI("Display", "Showing Blank.");
    return (xQueueSendToBack(display_queue, &packet, 50 / portTICK_PERIOD_MS) == pdTRUE);
}

bool Display::pause() {
    if(xSemaphoreTake(display_semaphore, 50 / portTICK_PERIOD_MS) != pdTRUE) return false;
    return true;
}

void Display::resume() {
    xSemaphoreGive(display_semaphore);

    return;
}


void displayTask(void* pvParameters) {
    Display* display = (Display*) pvParameters;
    ESP_LOGI("RTOS", "Display Task Started.");

    uint32_t start_tm = 0;
    uint8_t animation_state = 0;
    uint32_t delay_tm = 0;
    bool go_to_next_screen = true;
    bool check_next = false;

    enum QueueCheckType {
        BLOCK,
        TIMEOUT,
        SKIP
    } check_type = SKIP;

    while(1) {
        {
            Display::DisplayQueuePacket packet;

            switch (check_type) {
                case BLOCK:
                    if (xQueueReceive(display -> display_queue, &packet, portMAX_DELAY) == pdFALSE) continue; // Wait till we have a screen to display
                    display -> state = packet.state;
                case TIMEOUT:
                    if (xQueueReceive(display -> display_queue, &packet, 50 / portTICK_PERIOD_MS) == pdTRUE ) {
                        display -> state = packet.state;// Check if loading completed yet.
                    }
                    break;
                case SKIP:
                    break;
            }

        }

        ESP_LOGI("DISPLAY", "State: %d, Stack: %d", display -> state, uxTaskGetStackHighWaterMark(NULL));

        if (delay_tm > 0) vTaskDelay(delay_tm / portTICK_PERIOD_MS);
        xSemaphoreTake(display -> display_semaphore, portMAX_DELAY);

        start_tm = millis();
        
        display -> oled.firstPage();

        /* Display Frame Rendering State Machine. */
        switch ( display -> state ) {
            case Display::DISPLAY_LOADING_WHEEL: {
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
                    display -> oled.drawXBMP(0, 0, 128, 64, bitmap);
                } while(display -> oled.nextPage());

                break;
            }

            case Display::DISPLAY_LOADED_ANIMATION: {
                // Allocate memory for the combined bitmap
                #ifdef BOARD_HAS_PSRAM
                    uint8_t* bitmap = (uint8_t*) heap_caps_malloc(sizeof(uint8_t) * 128 * 64, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
                #else
                    uint8_t* bitmap = (uint8_t*) malloc(sizeof(uint8_t) * 128 * 64);
                #endif

                switch( animation_state ) {
                    case 0:
                        ESP_LOGI("Display", "Intermediate loading screen 0.");
                        display -> combineBitMaps(bitmap ,128 * 64, 2, (uint8_t*) &epd_bitmap_Loading_Screen_1, (uint8_t*) &epd_bitmap_Loading_Screen_3);
                        break;
                    case 1:
                        ESP_LOGI("Display", "Intermediate loading screen 1.");
                        display -> combineBitMaps(bitmap, 128 * 64, 2, (uint8_t*) &epd_bitmap_Loading_Screen_1, (uint8_t*) &epd_bitmap_Loading_Screen_2);
                        break;
                    case 2:
                        ESP_LOGI("Display", "Intermediate loading screen 2.");
                        display -> combineBitMaps(bitmap, 128 * 64, 2, (uint8_t*) &epd_bitmap_Loading_Screen_2, (uint8_t*) &epd_bitmap_Loading_Screen_3);
                        break;
                    default:
                        ESP_LOGI("Display", "Intermediate loading screen.");
                        display -> combineBitMaps(bitmap, 128 * 64, 3, (uint8_t*) &epd_bitmap_Loading_Screen_1, (uint8_t*) &epd_bitmap_Loading_Screen_2, (uint8_t*) &epd_bitmap_Loading_Screen_3);
                        break;
                }
                
                if (bitmap != nullptr) {
                    do {
                        display -> oled.drawXBMP(0, 0, 128, 64, bitmap);
                    } while(display -> oled.nextPage());

                    ESP_LOGI("Display", "Loaded.");
                    free(bitmap);
                    break;
                }

                ESP_LOGE("Display", "Failed to allocate.");
                break;

            }

            case Display::DISPLAY_SHOW_SPLASH:
                do {
                    display -> drawSplashFrame();
                } while (display -> oled.nextPage());
                break;

            case Display::DISPLAY_SHOW_BLANK:
                display -> oled.clear();
                break;

            case Display::DISPLAY_SHOW_SUMMARY:
                do {
                    display -> drawSummaryFrame();
                } while (display -> oled.nextPage());
                break;
            
            default:
            break;
        }

        uint32_t elapsed_tm = millis() - start_tm;


        /* Animation logic state machine. */
        switch( display -> state ) {
            case Display::DISPLAY_LOADING_WHEEL:
                check_type = TIMEOUT;

                if (elapsed_tm < LOADING_SCROLL_SPEED) { // Wait till next frame.
                    delay_tm = LOADING_SCROLL_SPEED - elapsed_tm;
                } else delay_tm = 0;

                if (animation_state >= 2) { // Loop around.
                    animation_state = 0;
                    break;
                }

                animation_state++;
                
                break;
            case Display::DISPLAY_LOADED_ANIMATION:
                if (elapsed_tm < LOADING_SCROLL_SPEED) {
                    delay_tm = LOADING_SCROLL_SPEED - elapsed_tm;
                } else delay_tm = 0;
                
                if (animation_state < 3) {
                    animation_state = 3;
                    check_type = SKIP;
                    break;
                }

                check_type = BLOCK; // Loading Screen animation finished playing, get the next screen.
                break;

            case Display::DISPLAY_SHOW_SPLASH:
                display -> state = Display::DISPLAY_LOADING_WHEEL; // Show loading wheel
                check_type = SKIP;
                if (elapsed_tm > SPLASH_DURATION_MS) {
                    delay_tm = 0;
                    break;
                }
                delay_tm = SPLASH_DURATION_MS - elapsed_tm;
                break;

            case Display::DISPLAY_SHOW_SUMMARY:
                
                check_type = TIMEOUT;
                if (elapsed_tm > SUMMARY_POLL_RATE_MS) {
                    delay_tm = 0;
                    break;
                }
                delay_tm = SUMMARY_POLL_RATE_MS - elapsed_tm;
                break;

            default:
                if (elapsed_tm < LOADING_SCROLL_SPEED) {
                    delay_tm = LOADING_SCROLL_SPEED - elapsed_tm;
                } else delay_tm = 0;
                check_type = TIMEOUT;
                break;
        }
        
        ESP_LOGI("DISPLAY", "Updated display. Took: %u, Delay: %d, Check Type:%d", elapsed_tm, delay_tm, check_type);
        xSemaphoreGive(display -> display_semaphore);

    }

    ESP_LOGE("Display", "Deleted self.");
    vTaskDelete(NULL);
}

void Display::drawSummaryFrame() {
    oled.setBitmapMode(1);
    oled.drawFrame(1, 1, 126, 11);
    oled.drawLine(1, 14, 1, 18);
    oled.drawLine(126, 14, 126, 18);
    oled.drawLine(32, 14, 32, 16);
    oled.drawLine(64, 14, 64, 16);
    oled.drawLine(98, 14, 98, 16);
    
    uint8_t nmd_bar_width = map(summary_data -> total_apparent_power, 0, summary_data -> nmd, 0, 125);
    oled.drawBox(3, 3, nmd_bar_width, 7);
    oled.setFont(u8g2_font_profont22_tr);

    if ((summary_data->total_apparent_power) >= 10000) {
        drawStrf(2, 44, "%05.0fVA", (summary_data->total_apparent_power));
    } else if ((summary_data->total_apparent_power) >= 1000) {
        drawStrf(2, 44, "%04.1fVA", (summary_data->total_apparent_power));
    } else {
        drawStrf(2, 44, "%03.2fVA", (summary_data->total_apparent_power));
    }
    
    oled.setFont(u8g2_font_helvB08_tr);
    drawStrf(100, 44, "%01.3f", (summary_data->mean_power_factor));
    oled.setFont(u8g2_font_haxrcorp4089_tr);
    drawStrf(3, 54, "%03.2fV", (summary_data->mean_voltage));
    oled.drawStr(1, 28, "0                      %NMD                       1");
    drawStrf(3, 63, "%d/%d", (summary_data->on_modules), (summary_data->total_modules));
    drawStrf(51, 56, "%.2fHz", (summary_data->mean_frequency));
    oled.setFont(u8g2_font_4x6_tr);

    struct tm timeinfo;
    getLocalTime(&timeinfo);
    
    drawStrf(51, 63, "%02d:%02d:%02d %d/%d/%d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec, timeinfo.tm_mday, timeinfo.tm_mon, ((timeinfo.tm_year) - 100) + 2000);
    

    drawSignalStrengthIndicator();
}



void Display::drawSignalStrengthIndicator() {
    if (summary_data->connection_strength == 0) return; // Signal is Non-Existent.

    if (summary_data->connection_strength > -80) { // Signal is Not Good.
        oled.drawBox(102, 53, 2, 3);
    }

    if (summary_data->connection_strength > -70) { // Signal is Okay.
        oled.drawBox(106, 51, 2, 5);
    }

    if (summary_data->connection_strength > -67) { // Signal is Good.
        oled.drawBox(110, 49, 2, 7);
    }

    if (summary_data->connection_strength > -55) { // Signal is Very Good.
        oled.drawBox(114, 47, 2, 9);
    }
}

void Display::drawStrf(uint8_t x, uint8_t y, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char* str = (char*) malloc(sizeof(char) * 128);
    vsnprintf(str, 128, format, args);
    oled.drawStr(x, y, str);

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
 */
void Display::combineBitMaps(uint8_t* bitmap, size_t size, uint8_t num_bitmaps, ...) {
    va_list args;
    va_start(args, num_bitmaps);

    // Validate input arguments
    if (num_bitmaps == 0) {
        return; // No bitmaps to combine
    }

    // Check for memory allocation failure
    if (bitmap == nullptr) {
        return; // Allocation failed
    }

    // Initialize the combined bitmap with the first bitmap
    uint8_t* bmp_0 = va_arg(args, uint8_t*);
    memcpy(bitmap, bmp_0, size);

    // Combine the remaining bitmaps
    for (int i = 1; i < num_bitmaps; i++) {
        uint8_t* new_bitmap = va_arg(args, uint8_t*);
        for (uint32_t j = 0; j < (size); j++) {
            *(bitmap + j) |= *(new_bitmap + j);
        }
    }

    va_end(args);
    
    return;
}

void Display::drawSplashFrame() {
    oled.setBitmapMode(1);

    oled.setFont(u8g2_font_profont22_tr);
    oled.drawStr(17, 15, "S");
    oled.drawStr(28, 32, "D");
    oled.drawStr(39, 49, "R");

    oled.setFont(u8g2_font_helvB08_tr);
    oled.drawStr(29, 15, "MART");
    oled.drawStr(40, 32, "EMAND");
    oled.drawStr(51, 49, "ESPONSE");

    oled.setFont(u8g2_font_4x6_tr);
    drawStrf(1, 63, "Version: %s", version);
}