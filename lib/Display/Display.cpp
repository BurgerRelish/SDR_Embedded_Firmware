#include "Display.h"
#include "epd_bitmap.h"

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

void Display::drawSummaryFrame() {
    oled.setBitmapMode(1);
    // Draw date & time.
    struct tm timeinfo;
    getLocalTime(&timeinfo);
    oled.setFont(u8g2_font_4x6_tr);
    drawStrf(40, 10, "%02d/%02d/%02d", timeinfo.tm_mday, timeinfo.tm_mon + 1, ((timeinfo.tm_year) - 100));
    oled.setFont(u8g2_font_haxrcorp4089_tr);
    drawStrf(1, 10, "%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);

    // Draw  Information
    oled.setFont(u8g2_font_haxrcorp4089_tr);
    drawStrf(8, 62, "%.2fV", summary_data -> mean_voltage);
    drawStrf(48, 62, "%.2fHz", summary_data -> mean_frequency);
    drawStrf(90, 62, "PF%.3f", (1 + summary_data -> mean_power_factor));
    
    oled.setFont(u8g2_font_profont22_tr);
    if ((summary_data->total_apparent_power) >= 10000) {
        drawStrf(0, 51, "%.0fVA", (summary_data->total_apparent_power));
    } else if ((summary_data->total_apparent_power) >= 1000) {
        drawStrf(0, 51, "%.1fVA", (summary_data->total_apparent_power));
    } else {
        drawStrf(0, 51, "%.2fVA", (summary_data->total_apparent_power));
    }

    oled.setFont(u8g2_font_4x6_tr);
    oled.drawStr(95, 42, "MODULES");
    oled.setFont(u8g2_font_haxrcorp4089_tr);
    drawStrf(93, 51, "%02d/%02d", (summary_data->on_modules), (summary_data->total_modules));

    drawSignalStrengthIndicator();

    // Draw Icons
    if ( summary_data -> power_status ) oled.drawXBMP( -4, 51, 16, 16, image_Voltage_16x16_bits); // On mains
    else oled.drawXBMP( -5, 50, 16, 16, image_Battery_16x16_bits); // On battery

    oled.drawXBMP( 94, 0, 10, 10, image_loading_10px_bits);
    oled.drawXBMP( 83, 2, 9, 8, image_Alert_9x8_bits);
    oled.drawXBMP( 105, 6, 7, 4, image_ButtonDown_7x4_bits);
    oled.drawXBMP( 105, 0, 7, 4, image_ButtonUp_7x4_bits);

    

    // Draw Demand Bar
    oled.setFont(u8g2_font_haxrcorp4089_tr);
    oled.drawStr(45, 21, "DEMAND");
    oled.drawFrame(0, 22, 127, 11);
    oled.drawBox(2, 24, (uint8_t) map(summary_data -> total_apparent_power, 0, summary_data -> nmd, 0, 124), 7);
    oled.drawLine(1, 16, 1, 20);
    oled.drawLine(127, 16, 127, 20);
    oled.drawLine(32, 18, 32, 20);
    oled.drawLine(96, 18, 96, 20);
    oled.setFont(u8g2_font_4x6_tr);
    oled.drawStr(3, 21, "0");
    oled.drawStr(114, 21, "MAX");
}



void Display::drawSignalStrengthIndicator() {
    if (summary_data->connection_strength == 0) return; // Signal is Non-Existent.

    if (summary_data->connection_strength > -80) { // Signal is Not Good.
        oled.drawBox(116, 7, 2, 3);
    }

    if (summary_data->connection_strength > -70) { // Signal is Okay.
        oled.drawBox(119, 5, 2, 5);
    }

    if (summary_data->connection_strength > -67) { // Signal is Good.
        oled.drawBox(122, 3, 2, 7);
    }

    if (summary_data->connection_strength > -55) { // Signal is Very Good.
        oled.drawBox(125, 1, 2, 9);
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