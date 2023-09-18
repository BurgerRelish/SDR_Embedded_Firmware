#include <stdarg.h>
#include <esp_heap_caps.h>
#include <time.h>

#include "Display.h"
#include "epd_bitmap.h"

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