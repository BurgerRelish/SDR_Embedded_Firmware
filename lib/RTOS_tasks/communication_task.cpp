#include "communication_task.h"

#include "SDRApp.h"
#include <memory>

#include "config.h"
#include "VariableDelay.h"

#include <WiFi.h>
#include "../Communication/MessageSerializer.h"
#include "../Communication/MessageDeserializer.h"
#include "../Communication/MQTTClient.h"

#include "Persistence.h"

MQTTClient* mqtt_client;
MessageSerializer* serializer;

#define TARGET_LOOP_FREQUENCY 1
void handleCommsMessage();
void handleMQTTMessage(MessageDeserializer* data);

void commsTaskFunction(void* global_class) {
    std::shared_ptr<SDR::AppClass> app;
    {
        auto appClass = static_cast<SDR::AppClass*>(global_class);
        app = appClass -> get_shared_ptr();
    }

    xSemaphoreTake(app -> control_task_semaphore, portMAX_DELAY);
    xSemaphoreGive(app -> control_task_semaphore);
    app -> setStatusLEDState(STATUS_LED_CONNECTING);
    //MQTTClient mqtt_client;
    WiFiClient wifi_client;

    WiFi.setAutoReconnect(true);
    WiFi.setHostname(WIFI_HOSTNAME);

    /* Start WiFi Connection. */
    {
        auto filesys = app -> get_fs();
        {
            Persistence<fs::LittleFSFS> nvs(filesys.data(), "/conn.txt", 1024);
            if (nvs.document["type"] == "wifi") {
                WiFi.begin(nvs.document["ssid"].as<std::string>().c_str(), (nvs.document["enc"] == "WPA2") ? nvs.document["pass"].as<std::string>().c_str() : (const char*) nullptr);
            }
        }
    }

    {
        uint64_t wifi_start_time = esp_timer_get_time();
        while(WiFi.status() != WL_CONNECTED) {
            if ((esp_timer_get_time() - wifi_start_time) / 1000 > WIFI_TIMEOUT_MS) throw SDR::Exception("WiFi Connect Timed out.");
            if (WiFi.status() == WL_CONNECT_FAILED) throw SDR::Exception("WiFi Connect Failed.");
            vTaskDelay(10/portTICK_PERIOD_MS);
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
                topic_list.push_back(v.as<ps_string>());
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
    
    //if(!mqtt_client.begin()) throw SDR::Exception("MQTT Client not ready.");

    VariableDelay vd("RE", TARGET_LOOP_FREQUENCY); // Create a new variable delay class to set target frequency.

    vd.addCallback(handleCommsMessage, 50);
    while(1) {
        xSemaphoreTake(app -> control_task_semaphore, portMAX_DELAY);
        xSemaphoreGive(app -> control_task_semaphore);

        

        vd.loop();
    }

    vTaskDelete(NULL);
}


void handleCommsMessage() {

}

void handleMQTTMessage(MessageDeserializer* data) {

}
