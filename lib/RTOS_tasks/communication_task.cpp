#include "communication_task.h"

#include "SDRApp.h"
#include <memory>
#include "FS.h"
#include "LittleFS.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#include "config.h"
#include "VariableDelay.h"


#include "../Communication/MessageSerializer.h"
#include "../Communication/MessageDeserializer.h"
#include "../Communication/MQTTClient.h"

#include "Persistence.h"

#include "../webpages/webpage_setup.h"

MQTTClient* mqtt_client;
MessageSerializer* serializer;

#define TARGET_LOOP_FREQUENCY 1
void handleCommsMessage();
void handleMQTTMessage(MessageDeserializer* data);

void connectWiFi(Persistence<fs::LittleFSFS>&);
void startWebServerSetup(fs::LittleFSFS&&);
bool checkSetupParameters(AsyncWebServerRequest *);

void commsTaskFunction(void* global_class) {
    std::shared_ptr<SDR::AppClass> app;
    {
        auto appClass = static_cast<SDR::AppClass*>(global_class);
        app = appClass -> get_shared_ptr();
    }

    xSemaphoreTake(app -> comms_task_semaphore, portMAX_DELAY);
    xSemaphoreGive(app -> comms_task_semaphore);
    app -> setStatusLEDState(STATUS_LED_CONNECTING);
    //MQTTClient mqtt_client;
    

    /* Start WiFi Connection. */
    uint8_t conn_type = 0;
    {
        auto filesys = app -> get_fs();
        //if (!filesys.data().exists("/conn.txt") || !filesys.data().exists("/mqtt.txt")) startWebServerSetup(filesys.data());
        {
            Persistence<fs::LittleFSFS> nvs(filesys.data(), "/conn.txt", 1024, true);
            
            if (!nvs.document.containsKey("type")) {
                nvs.document["type"] = "wifi";
                nvs.document["enc"] = "WPA2";
                nvs.document["ssid"] = "Routy";
                nvs.document["pass"] = "0609660733";
                ESP_LOGI("WiFi", "Loaded defaults.");
            }

            if (nvs.document["type"] == "wifi") {
                conn_type = 1;
                connectWiFi(nvs);
            }
        }
    }

    /* Connect to MQTT Broker */
    {
        auto filesys = app -> get_fs();
        {
            Persistence<fs::LittleFSFS> nvs(filesys.data(), "/mqtt.txt", 1024);

            auto array = nvs.document["topics"].as<JsonArray>();
            ps_vector<ps_string> topic_list;
            for (auto v : array) {
                ps_string topic = v.as<ps_string>();
                ESP_LOGI("MQTT", "Loaded Topic: %s", topic.c_str());
                topic_list.push_back(topic);
            }

        //     mqtt_client = MQTTClient(wifi_client, 
        //                             handleMQTTMessage,
        //                             nvs.document["server"].as<ps_string>(),
        //                             nvs.document["port"].as<uint32_t>(),
        //                             nvs.document["username"].as<ps_string>(),
        //                             nvs.document["password"].as<ps_string>(), 
        //                             topic_list);
        //
         }
    }
    SentryQueueMessage msg;
    msg.data = nullptr;
    msg.new_state = COMMS_SETUP_COMPLETE;
    xQueueSend(app -> sentry_task_queue, &msg, portMAX_DELAY);
    vTaskSuspend(NULL);
    //if(!mqtt_client.begin()) throw SDR::Exception("MQTT Client not ready.");

    VariableDelay vd("RE", TARGET_LOOP_FREQUENCY); // Create a new variable delay class to set target frequency.

    vd.addCallback(handleCommsMessage, 50);

    while(1) {
        xSemaphoreTake(app -> comms_task_semaphore, portMAX_DELAY);
        xSemaphoreGive(app -> comms_task_semaphore);

        

        vd.loop();
    }

    vTaskDelete(NULL);
}


void handleCommsMessage() {
    return;
}

void handleMQTTMessage(MessageDeserializer* data) {

}

void connectWiFi(Persistence<fs::LittleFSFS>& nvs) {
    WiFi.setAutoReconnect(true);
    WiFi.setHostname(WIFI_HOSTNAME);

    auto ssid = nvs.document["ssid"].as<std::string>();
    auto password = (nvs.document["enc"] == "WPA2") ? nvs.document["pass"].as<std::string>() : std::string();
    ESP_LOGI("WiFi", "Connecting to [%s, %s]", ssid.c_str(), password.c_str());
    WiFi.begin(ssid.c_str(), password.c_str());

    uint64_t wifi_start_time = esp_timer_get_time();
    while(WiFi.status() != WL_CONNECTED) {
        if ((esp_timer_get_time() - wifi_start_time) / 1000 > WIFI_TIMEOUT_MS) throw SDR::Exception("WiFi Connect Timed out.");
        if (WiFi.status() == WL_CONNECT_FAILED) throw SDR::Exception("WiFi Connect Failed.");
        vTaskDelay(10/portTICK_PERIOD_MS);
    }

    ESP_LOGI("WiFi", "Connected.");
}


void startWebServerSetup(fs::LittleFSFS& file_system) {
    WiFi.mode(WIFI_MODE_APSTA);
    WiFi.softAP(WIFI_HOSTNAME, WIFI_SETUP_AP_PASSWORD);
    AsyncWebServer server(80);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(200, "text/html", setup_html);
    });

    std::string url = "/save";
    server.on(url.c_str(),
        HTTP_POST,
        [&file_system](AsyncWebServerRequest *request) {
            if (checkSetupParameters(request)) {
                Persistence<fs::LittleFSFS> nvs(file_system, "/conn.txt", 1024, true);
                
            }

            request->send(200, "text/plain", "");
        }
    );

}

bool checkSetupParameters(AsyncWebServerRequest *request) {
    return (request -> hasParam("ssid") && 
            request -> hasParam("password") &&
            request -> hasParam("connectivityType"));
}
