#include "tasks.h"

#include "App.h"
#include <memory>
#include "FS.h"
#include "LittleFS.h"
#include <WiFi.h>

#include "config.h"
#include "VariableDelay.h"
#include "Persistence.h"

#include "../Serialization/MessageSerializer.h"
#include "../Serialization/MessageDeserializer.h"
#include "../Communication/MQTTClient.h"

#include <ps_stl.h>

std::shared_ptr<MQTTClient> mqtt_client;
WiFiClient wifi_client;
ps::queue<std::shared_ptr<MessageDeserializer>> mqtt_incoming_messages;

#define TARGET_LOOP_FREQUENCY 20 // Hz
void handleCommsMessage();

void sendReadings(std::shared_ptr<sdr::App>);
void checkUpdates(std::shared_ptr<sdr::App>);

void callbackMQTTMessage(std::shared_ptr<MessageDeserializer> msg) {
    mqtt_incoming_messages.push(msg);
}

void handleMQTTMessage(std::shared_ptr<sdr::App>);
void loadUpdate(std::shared_ptr<MessageDeserializer>&, std::shared_ptr<sdr::App>&);
void loadUnitCommand(std::shared_ptr<MessageDeserializer>&, std::shared_ptr<sdr::App>&);
void loadModuleCommand(std::shared_ptr<MessageDeserializer>&, std::shared_ptr<sdr::App>&);

void connectWiFi(Persistence<fs::LittleFSFS>&);
// void startWebServerSetup(fs::LittleFSFS&&);
// bool checkSetupParameters(AsyncWebServerRequest *);

void commsTaskFunction(void* global_class) {
    std::shared_ptr<sdr::App> app;
    {
        auto appClass = static_cast<sdr::App*>(global_class);
        app = appClass -> get_shared_ptr();
    }

    xSemaphoreTake(app -> comms_task_semaphore, portMAX_DELAY);
    xSemaphoreGive(app -> comms_task_semaphore);
    app -> setStatusLEDState(STATUS_LED_CONNECTING);
    

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

    try {
         {
        auto filesys = app -> get_fs();
        {
            {
                Persistence<fs::LittleFSFS> nvs(filesys.data(), "/mqtt.txt", 1024, true);
                nvs.document.clear();
                if (!nvs.document.containsKey("server")) {

                    nvs.document["server"] = "13.245.36.134";
                    nvs.document["port"] = 1883;
                    nvs.document["username"] = "72a6fdf8-4b3a-49dd-bb12-ae8b93b02807";
                    nvs.document["password"] = "2YUYzeu.F7-X9r#$";

                    auto ingress_topics = nvs.document.createNestedArray("ingress");
                    ingress_topics.add("/egress/stellenbosch");
                    ingress_topics.add("/egress/devices/72a6fdf8-4b3a-49dd-bb12-ae8b93b02807");
                    
                    auto egress_topics = nvs.document.createNestedArray("egress");
                    egress_topics.add("/ingress/stellenbosch");

                    ESP_LOGI("MQTT", "Loaded defaults.");
                }
            }

            Persistence<fs::LittleFSFS> nvs(filesys.data(), "/mqtt.txt", 1024);
            ps::vector<ps::string> ingress_topics;
            ps::vector<ps::string> egress_topics;

            
            {  
                ESP_LOGV("MQTT", "Loading Ingress Topics...");
                auto ingress_topic_list = nvs.document["ingress"].as<JsonArray>();
                for (auto ingress_topic : ingress_topic_list) {
                    ps::string topic = ingress_topic.as<ps::string>();
                    ESP_LOGV("MQTT", "Loaded Ingress Topic: %s", topic.c_str());
                    ingress_topics.push_back(topic);
                }

                ESP_LOGV("MQTT", "Loading Egress Topics...");
                auto egress_topic_list = nvs.document["egress"].as<JsonArray>();
                for (auto egress_topic : egress_topic_list) {
                    ps::string topic = egress_topic.as<ps::string>();
                    ESP_LOGV("MQTT", "Loaded Egress Topic: %s", topic.c_str());
                    egress_topics.push_back(topic);
                }
            }

            ESP_LOGV("MQTT", "Creating MQTTClient.");
            mqtt_client = ps::make_shared<MQTTClient>(wifi_client, 
                                    callbackMQTTMessage,
                                    nvs.document["server"].as<ps::string>(),
                                    nvs.document["port"].as<uint32_t>(),
                                    nvs.document["username"].as<ps::string>(),
                                    nvs.document["password"].as<ps::string>(), 
                                    ingress_topics,
                                    egress_topics);
        
         }
    }
    } catch (std::exception& e) {
        ESP_LOGE("MQTT", "Except: %s", e.what());
    }

    /* Connect to MQTT Broker */
    if(!mqtt_client -> begin()) throw sdr::Exception("MQTT Client not ready.");
    
    SentryQueueMessage msg;
    msg.data = nullptr;
    msg.new_state = COMMS_SETUP_COMPLETE;
    xQueueSend(app -> sentry_task_queue, &msg, portMAX_DELAY);
    vTaskSuspend(NULL);

    VariableDelay vd("RE", TARGET_LOOP_FREQUENCY); // Create a new variable delay class to set target frequency.

    vd.addCallback(handleCommsMessage, 50);
    vd.addCallback([app](){sendReadings(app);}, 60000 * READING_INTERVAL_MIN); // Create readings interval.
    vd.addCallback([app](){checkUpdates(app);}, 60000 * UPDATE_REQUEST_INTERVAL_MIN);

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

void handleMQTTMessage(std::shared_ptr<sdr::App> app) {
    while (!mqtt_incoming_messages.empty()) {
        auto& data = mqtt_incoming_messages.front();

        if (!data -> document.containsKey("type")) {
            ESP_LOGE("MQTT", "Invalid message format. [type required].");
            return;
        }

        auto msg_type = data -> document["type"].as<ps::string>();

        if (msg_type == "update") {
            loadUpdate(data, app);
        } else if (msg_type == "unit_command") {
            loadUnitCommand(data, app);
        } else if (msg_type == "module_command") {
            loadModuleCommand(data, app);
        } else ESP_LOGE("MQTT", "Unknown message type received.");

        mqtt_incoming_messages.pop();
    }

    return;
}

void checkUpdates(std::shared_ptr<sdr::App> app) {
    auto modules = app -> get_modules();

    MessageSerializer serializer(mqtt_client, 0, 2048);
    serializer.document["type"] = "update";

    auto module_id_arr = serializer.document.createNestedArray("moduleID");
    for (auto module : modules.data()) {
        if (module -> updateRequired()) {
            module_id_arr.add(module -> getModuleID().c_str());
        }
    }
    
    return;
}

void sendReadings(std::shared_ptr<sdr::App> app) {
    uint16_t serialization_count = 0;
    ps::string packet_str;
    WiFi.RSSI();
    
    auto modules = app -> get_modules();
    MessageSerializer serializer(mqtt_client, 0, 65535);
    serializer.document["type"] = "reading";

    auto reading_arr = serializer.document.createNestedArray("readings");
    auto status_change_arr = serializer.document.createNestedArray("statusUpdates");
    for (auto& module : modules.data()) {
        module -> serialize(reading_arr);
    }

    return;
}

void loadUpdate(std::shared_ptr<MessageDeserializer>& data, std::shared_ptr<sdr::App>& app){
    {    
        auto unit_object = data -> document["unit"].as<JsonObject>();
        auto unit = app -> get_unit();
        unit.data().loadUpdate(unit_object);
    }

    auto module_arr = data -> document["modules"].as<JsonArray>();
    auto module_map = app -> get_module_map();
    for (auto module_obj : module_arr) {
        /* Confirm that at least the module ID is present. */
        if (!module_obj.containsKey("moduleID")) {
            ESP_LOGE("MODULE", "Missing ID.");
            continue;
        }

        /* Find the module by its ID. */
        ps::string module_id = module_obj["moduleID"].as<ps::string>();
        auto module_it = module_map.data().find(module_id.c_str());

        if (module_it == module_map.data().end()) {
            ESP_LOGI("MODULE", "Unknown ID.");
            continue;
        }

        /* Load the updated parameters into the module. */
        //module_it -> second -> loadUpdate(module_obj.as<JsonObject>());
    }
}

void loadUnitCommand(std::shared_ptr<MessageDeserializer>& data, std::shared_ptr<sdr::App>& app){
    ControlQueueMessage msg;
    msg.type = CTRL_UNIT_COMMAND;
    msg.data = data;
    xQueueSend(app -> control_task_queue, &msg, portMAX_DELAY);
}

void loadModuleCommand(std::shared_ptr<MessageDeserializer>& data, std::shared_ptr<sdr::App>& app){
    ControlQueueMessage msg;
    msg.type = CTRL_MODULE_COMMAND;
    msg.data = data;
    xQueueSend(app -> control_task_queue, &msg, portMAX_DELAY);
}

void connectWiFi(Persistence<fs::LittleFSFS>& nvs) {
    WiFi.mode(WIFI_MODE_STA);
    WiFi.setAutoReconnect(true);
    WiFi.setHostname(WIFI_HOSTNAME);

    auto ssid = nvs.document["ssid"].as<std::string>();
    auto password = (nvs.document["enc"] == "WPA2") ? nvs.document["pass"].as<std::string>() : std::string();
    ESP_LOGI("WiFi", "Connecting to [%s, %s]", ssid.c_str(), password.c_str());
    WiFi.begin(ssid.c_str(), password.c_str());

    uint64_t wifi_start_time = esp_timer_get_time();
    while(WiFi.status() != WL_CONNECTED) {
        if ((esp_timer_get_time() - wifi_start_time) / 1000 > WIFI_TIMEOUT_MS) throw sdr::Exception("WiFi Connect Timed out.");
        if (WiFi.status() == WL_CONNECT_FAILED) throw sdr::Exception("WiFi Connect Failed.");
        vTaskDelay(10/portTICK_PERIOD_MS);
    }

    ESP_LOGI("WiFi", "Connected.");
}


// void startWebServerSetup(fs::LittleFSFS& file_system) {
//     WiFi.mode(WIFI_MODE_APSTA);
//     WiFi.softAP(WIFI_HOSTNAME, WIFI_SETUP_AP_PASSWORD);
//     AsyncWebServer server(80);

//     server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
//         request->send(200, "text/html", setup_html);
//     });

//     std::string url = "/save";
//     server.on(url.c_str(),
//         HTTP_POST,
//         [&file_system](AsyncWebServerRequest *request) {
//             if (checkSetupParameters(request)) {
//                 Persistence<fs::LittleFSFS> nvs(file_system, "/conn.txt", 1024, true);
                
//             }

//             request->send(200, "text/plain", "");
//         }
//     );

// }

// bool checkSetupParameters(AsyncWebServerRequest *request) {
//     return (request -> hasParam("ssid") && 
//             request -> hasParam("password") &&
//             request -> hasParam("connectivityType"));
// }
