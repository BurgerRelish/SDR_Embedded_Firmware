#include "sentry_task.h"
#include "config.h"

#include <FastLED.h>
#include <WiFi.h>
#include <Preferences.h>
#include <string>
#include <sstream>
#include <HTTPClient.h>

#include "pin_map.h"
#include "VariableDelay.h"
#include "../device_identifiers.h"
#include "../data_containers/ps_string.h"
#include "../Communication/MessageDeserializer.h"
#include "../sdr_containers/SDRUnit.h"

#include "../hardware_interface/InterfaceMaster.h"

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


/* Dynamic Setup */
struct unitSetupParameters {
    ps_string broker_url;
    ps_string broker_port;
    ps_string mqtt_password;
    ps_string mqtt_ingress_topic;
    ps_string mqtt_area_topic;
    ps_string mqtt_device_topic;
};

bool checkSetup();
unitSetupParameters fetchUnitParameters();
/* ============= */

/* Status LED Functions */
CRGB status_led[1];
uint8_t gHue = 0;
#define deltaHue 1
#define BRIGHTNESS 25

void initFastLED();
void updateRainbow();
void setStatic(const uint8_t hue);
/* ==================== */

bool startSentryTask() {
    if(xTaskCreate(
        sentryTaskFunction,
        SENTRY_TASK_NAME,
        SENTRY_TASK_STACK,
        NULL,
        SENTRY_PRIORITY,
        &SentryTask
        ) == pdTRUE) return true;
    
    ESP_LOGE("SENTRY", "Failed to start sentry task.");
    return false;
}

void sentryTaskFunction(void* pvParameters) {
    initFastLED();
    setStatic((180/360) * 255); // Init the status led and set to light blue.

    connect();

    if(!checkSetup()) {
        ESP_LOGI("DEVICE", "Attempting to fetch setup parameters.");
        auto params = fetchUnitParameters(SETUP_API_URL, SETUP_API_PORT, UNIT_UID, UNIT_SETUP_KEY);
    }

    VariableDelay vd(SENTRY_TASK_NAME, TARGET_LOOP_FREQUENCY); // Create a new variable delay class to set target frequency.
    vd.addCallback(updateRainbow, 33);

    while(1) {
        vd.loop();
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

bool checkSetup() {
    nvs.begin("/SETUP");
    auto retval = nvs.getBool("SETUP");
    nvs.end();

    return retval;
}

unitSetupParameters fetchUnitParameters(const char* api_url, uint32_t api_port, const char* uid, const char* key) {
    HTTPClient client;
    std::ostringstream url;

    url << "http://" << api_url << ":" << api_port << "/setup/&uid=" << uid << "&key=" << key;
    ESP_LOGI("HTTP", "GETting Unit Parameters");
    log_printf("URL: %s\n", url.str().c_str());

    client.setURL(url.str().c_str());
    auto response_code = client.GET();

    if (response_code != 200) { // HTTP 200 - OK
        ESP_LOGE("HTTP", "Error in HTTP GET request: %d", response_code);
        client.end();
        return;
    }
    
    ps_string response_str = client.getString().c_str();
    log_printf("Got response: %s\n", response_str.c_str());

    client.end();

    MessageDeserializer params(response_str);
    unitSetupParameters ret;

    ret.broker_port = params["port"];
    ret.broker_url = params["url"];
    ret.mqtt_ingress_topic = params["itopic"];
    ret.mqtt_device_topic = params["dtopic"];
    ret.mqtt_area_topic = params["itopic"];
    ret.mqtt_password = params["pass"];

    return ret;
}


