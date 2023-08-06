#include "sentry_task.h"
#include "config.h"

#include <Arduino.h>

#include <FastLED.h>
#include <WiFi.h>
#include <Preferences.h>
#include <string>
#include <sstream>
#include <HTTPClient.h>

#include "SDRApp.h"

#include "VariableDelay.h"
#include "../data_containers/ps_string.h"
#include "../Communication/MessageDeserializer.h"
#include "../sdr_containers/SDRUnit.h"

#include "../hardware_interface/InterfaceMaster.h"

#define TARGET_LOOP_FREQUENCY 10

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

// void initMQTT();
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
// unitSetupParameters fetchUnitParameters();
/* ============= */

/* Status LED Functions */
CRGB status_led[1];
uint8_t gHue = 0;
#define deltaHue 1
#define BRIGHTNESS 25

// void initFastLED();
// void updateRainbow();
// void setStatic(const uint8_t hue);
/* ==================== */

void checkTaskStatus(TaskHandle_t task);

// void sentryPrintInfo(SDR::AppClass* app) {
//     ESP_LOGI("SENTRY", "==== Task Information ====");
//     log_printf("Sentry - State: %d, High Watermark: %u\n",eTaskGetState(app -> sentry_task_handle), uxTaskGetStackHighWaterMark2(app -> sentry_task_handle));
//     log_printf("Comms - State: %d, High Watermark: %u\n",eTaskGetState(app -> comms_task_handle), uxTaskGetStackHighWaterMark2(app -> comms_task_handle));
//     log_printf("Control - State: %d, High Watermark: %u\n",eTaskGetState(app -> control_task_handle), uxTaskGetStackHighWaterMark2(app -> control_task_handle));
//     log_printf("Rule Engine - State: %d, High Watermark: %u\n",eTaskGetState(app -> rule_engine_task_handle), uxTaskGetStackHighWaterMark2(app -> rule_engine_task_handle));
// }

void sentryTaskFunction(void* global_class) {
    std::shared_ptr<SDR::AppClass> app;
    {
        auto appClass = static_cast<SDR::AppClass*>(global_class);
        app = appClass -> get_shared_ptr();
    }

    try {
        xSemaphoreTake(app -> sentry_task_semaphore, portMAX_DELAY);
        xSemaphoreGive(app -> sentry_task_semaphore);
        app -> setStatusLEDState(STATUS_LED_SETUP);
        app -> updateStatusLED();

        /* Allow other RTOS tasks to start. */
        xSemaphoreGive(app -> control_task_semaphore);
        xSemaphoreGive(app -> rule_engine_task_semaphore);
        xSemaphoreGive(app -> comms_task_semaphore);

        VariableDelay vd(SENTRY_TASK_NAME, TARGET_LOOP_FREQUENCY); // Create a new variable delay class to set target frequency.
        // vd.addCallback([app](){sentryPrintInfo(app);}, 2500);

        while(1) {
            xSemaphoreTake(app -> sentry_task_semaphore, portMAX_DELAY);
            xSemaphoreGive(app -> sentry_task_semaphore);
            
            if(!app -> updateStatusLED()) ESP_LOGE("SENTRY", "Failed to update Status LED.");

            checkTaskStatus(app -> comms_task_handle);
            checkTaskStatus(app -> control_task_handle);
            checkTaskStatus(app -> rule_engine_task_handle);
            
            vd.loop();
            vd.delay();
        }
    } catch (SDR::Exception &e) {
        ESP_LOGE("SENTRY", "SDR Exception: %s", e.what());
    } catch (std::exception &e) {
        ESP_LOGE("SENTRY", "Exception: %s", e.what());
    }

    vTaskDelete(NULL);
}

void checkTaskStatus(TaskHandle_t task) {
    auto status = eTaskGetState(task);

    switch (status) {
        case eRunning:
            // Task is currently running
            break;
        case eReady:
            // Task is ready to run but not currently executing
            break;
        case eBlocked:
            // Task is blocked, waiting for a resource or event
            break;
        case eSuspended:
            // Task is suspended
            break;
        case eDeleted:
            // Task has been deleted
            break;
        case eInvalid:
            // Task handle is invalid or task does not exist
            break;
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

// void connect() {
//     ConnectivityType type = getConnectivityType();

//     if (type == CONNECT_WIFI || type == CONNECT_BOTH) {
//         initWiFi();
//     }

//     if (type == CONNECT_GSM || type == CONNECT_BOTH) {
//         initGSM();
//     }

//     if (type != CONNECT_NONE) {
//         initMQTT();
//     }
    
//     return;
// }

void initWiFi() {
    nvs.begin(CONNECTIVITY_STORAGE_NVS_PATH);
    String ssid = nvs.getString("ssid");
    String password = nvs.getString("pass");
    nvs.end();

    if(ssid.isEmpty()) {
        ESP_LOGI(SENTRY_TASK_NAME, "No stored WiFi credentials, using default.");
        ssid = "DEFAULT_WIFI_SSID";

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


