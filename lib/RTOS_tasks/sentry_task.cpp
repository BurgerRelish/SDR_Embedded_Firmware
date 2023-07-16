#include "sentry_task.h"
#include "config.h"

#include <FastLED.h>
#include <WiFi.h>
#include <Preferences.h>
#include <string>

#include "pin_map.h"
#include "VariableDelay.h"

#define TARGET_LOOP_FREQUENCY 30

void sentryTaskFunction(void* pvParameters);

/* NVS Functions */
Preferences nvs;

/* ============= */

/* Connectivity Functions */
enum ConnectivityType {
    CONNECT_WIFI,
    CONNECT_GSM,
    CONNECT_BOTH,
    CONNECT_NONE
};

ConnectivityType getConnectivityType();
void setConnectivityType(ConnectivityType type);
void connect();

void initWiFi();
void connectWiFi(const char* ssid, const char* password);
void storeWiFiCredentials(String ssid, String password);

void initGSM();
void storeGSMCredentials(String apn, String pin);

void initMQTT();
/* ====================== */


/* Status LED Functions */
CRGB status_led[1];
uint8_t gHue = 0;
#define deltaHue 1
#define BRIGHTNESS 25

void initFastLED();
void updateRainbow();
void setStatic(const uint8_t hue);
/* ==================== */

void startSentryTask() {
    xTaskCreate(
        sentryTaskFunction,
        SENTRY_TASK_NAME,
        SENTRY_TASK_STACK,
        NULL,
        SENTRY_PRIORITY,
        &SentryTask
        );
}

void sentryTaskFunction(void* pvParameters) {
    initFastLED();
    setStatic((180/360) * 255); // Init the status led and set to light blue.

    connect();

    VariableDelay vd(SENTRY_TASK_NAME, TARGET_LOOP_FREQUENCY); // Create a new variable delay class to set target frequency.
    while(1) {
        updateRainbow();

        vd.delay(); // Wait for remainder of frame to reach target frequency.
    }
}


void initFastLED() {
    FastLED.addLeds<WS2812, STATUS_LED_PIN, GRB>(status_led, 1);
    FastLED.setBrightness(BRIGHTNESS);
}

void updateRainbow() {
    status_led[0] = CHSV(gHue, 240, 255);
    FastLED.show();
    gHue += deltaHue;
}

void setStatic(const uint8_t hue) {
    if(hue != gHue) {
        gHue = hue;
        status_led[0] = CHSV(gHue, 240, 255);
        FastLED.show();
    }
}

ConnectivityType getConnectivityType() {
    nvs.begin(CONNECTIVITY_STORAGE_NVS_PATH);
    uint8_t type = nvs.getUInt("conntype");
    nvs.end();
    ESP_LOGI(SENTRY_TASK_NAME, "Got connectivity type: %d", type);
    return ConnectivityType(type);
}

void setConnectivityType(ConnectivityType type) {
    nvs.begin(CONNECTIVITY_STORAGE_NVS_PATH);
    nvs.putUInt("conntype", (uint32_t) type);
    nvs.end();
    ESP_LOGI(SENTRY_TASK_NAME, "Connectivity type set: %d", type);
}

void connect() {
    ConnectivityType type = getConnectivityType();

    if (type == CONNECT_WIFI || type == CONNECT_BOTH) {
        initWiFi();
    }

    if (type == CONNECT_GSM || type == CONNECT_BOTH) {
        initGSM();
    }

    if (type != CONNECT_NONE) {
        initMQTT();
    }
    
    return;
}

void initWiFi() {
    nvs.begin(CONNECTIVITY_STORAGE_NVS_PATH);
    String ssid = nvs.getString("ssid");
    String password = nvs.getString("pass");
    nvs.end();

    if(ssid.isEmpty()) {
        ESP_LOGI(SENTRY_TASK_NAME, "No stored WiFi credentials, using default.");
        ssid = DEFAULT_WIFI_SSID;

        storeWiFiCredentials(ssid, String());
    } 

    #ifdef DEBUG_SENTRY
    ESP_LOGD(SENTRY_TASK_NAME, "\n==== Loaded WiFi Credentials ====");
    log_printf("- SSID: %s", ssid.c_str());
    log_printf("\n- Password: %s", password.c_str());
    log_printf("\n=================================\n");
    #endif

    connectWiFi(ssid.c_str(), password.c_str());
}


void connectWiFi(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        ESP_LOGI(SENTRY_TASK_NAME, "Connecting to WiFi...");

        if (WiFi.status() == WL_CONNECT_FAILED) {
            ESP_LOGE(SENTRY_TASK_NAME, "Failed to connect to WiFi.");
        }

        vTaskDelay(1000/portTICK_PERIOD_MS);
    }
}

void storeWiFiCredentials(String ssid, String password) {
    nvs.begin(CONNECTIVITY_STORAGE_NVS_PATH);
    nvs.clear();
    nvs.putString("ssid", ssid);
    nvs.putString("pass", password);
    nvs.end();
}

void initGSM() {
    return;
}

void storeGSMCredentials(String apn, String pin) {
    nvs.begin(CONNECTIVITY_STORAGE_NVS_PATH);
    nvs.clear();
    nvs.putString("apn", apn);
    nvs.putString("pin", pin);
    nvs.end();
}


