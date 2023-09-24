#include <Arduino.h>
#include <ps_stl.h>
#include <esp_brotli.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <time.h>
#include <LittleFS.h>
#include "Config.h"
#include "PinMap.h"
#include "Display.h"
#include "MQTTClient.h"
#include "Persistence.h"

#include "webpage_setup.h"

#include "App/Unit.h"
#include "App/Functions.h"

void taskMain(void* pvParameters);
void taskComms(void* pvParameters);

WiFiClient wifi_client;

std::shared_ptr<re::FunctionStorage> functions;
std::shared_ptr<Unit> unit;
std::shared_ptr<Display> display;
std::shared_ptr<MQTTClient> mqtt_client;

ps::unordered_map<ps::string, std::shared_ptr<Module>> module_map;

TaskHandle_t main_task;
SemaphoreHandle_t main_task_semaphore;

SummaryFrameData* summary_data;

AsyncWebServer server(80);

void log_memory_usage() {
    static const size_t tot_psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    static const size_t tot_sram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);

    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);

    log_printf("- PSRAM Usage: %.4f/%u KB (%f%%)\n", ((float)(tot_psram - free_psram)) / 1024, tot_psram / 1024, 100 *( 1 - ((float) free_psram) / ((float) tot_psram)));
    log_printf("- SRAM Usage: %.4f/%u KB (%f%%)\n", ((float)(tot_sram - free_sram)) / 1024, tot_sram / 1024, 100 *( 1 - ((float) free_sram) / ((float) tot_sram)));
}

void serialize_readings();
void first_time_setup(Persistence& persistence);


void setup() {
  psramInit();
  if(!LittleFS.begin(true)) ESP_LOGE("LittleFS", "Failed to mount.");

  summary_data = (SummaryFrameData*) calloc(1, sizeof(SummaryFrameData));
  summary_data -> nmd = 1;

  display = ps::make_shared<Display>(DISPLAY_SCL, DISPLAY_SDA, VERSION);
  display -> begin(summary_data);
  ESP_LOGI("Display", "Started.");

  pinMode(U1_CTRL, OUTPUT);
  digitalWrite(U1_CTRL, HIGH);
  pinMode(U1_DIR, OUTPUT);
  digitalWrite(U1_DIR, HIGH);
  
  pinMode(U2_CTRL, OUTPUT);
  digitalWrite(U2_CTRL, HIGH);
  pinMode(U2_DIR, OUTPUT);
  digitalWrite(U2_DIR, HIGH);
  
  pinMode(STATUS_LED_PIN, OUTPUT);

  Serial1.begin(RS485_BAUD_RATE, SERIAL_8N1, U1_RXD, U1_TXD);
  Serial2.begin(RS485_BAUD_RATE, SERIAL_8N1, U2_RXD, U2_TXD);
  
  ESP_LOGI("PINS", "Modes set."); 

  main_task_semaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(main_task_semaphore);

  ESP_LOGI("RTOS", "Semaphores created.");

  xTaskCreate(
    taskMain,
    "Main Task",
    48 * 1024,
    NULL,
    5,
    &main_task
  );
}

void loop() {} // Loop is not used because of RTOS.

void taskMain(void* pvParameters) {
  ESP_LOGI("RTOS", "Main Task started.");

  { // Load WiFi parameters from flash.
    // Persistence persistence("/conn.txt", 8192, false); // Dont write anything to flash.
    // if (!persistence.document.containsKey("connect_type")) {
    //   ESP_LOGI("Connection", "Not configured. Starting setup page.");
    //   first_time_setup(persistence);
    // }

      WiFi.setAutoReconnect(true);
      // auto ssid = persistence.document["ssid"].as<ps::string>();
      // auto pass = persistence.document["password"].as<ps::string>();
      ps::string ssid = "Routy";
      ps::string pass = "0609660733";

      vTaskDelay(1000 / portTICK_PERIOD_MS);
      WiFi.begin(ssid.c_str(), pass.c_str());
      while (WiFi.status() != WL_CONNECTED) {
        log_memory_usage();
        vTaskDelay(500 / portTICK_PERIOD_MS);
        ESP_LOGI("WiFi","SSID: %s, Pass: %s", ssid.c_str(), pass.c_str());
    }

    configTime(GMT_OFFSET, 0, NTP_SERVER); // We are now connected to the internet, init time.

    ESP_LOGI("UNIT", "Starting MQTT Client.");
    mqtt_client = ps::make_shared<MQTTClient>(wifi_client); // Connect to MQTT

    ps::string auth_token = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJhZGRyZXNzIjoiMTkyLjE2OC4zLjIzIiwicG9ydCI6MTg4MywiZXhwIjoyMDEwOTI5OTkzLCJhY2wiOnsicHViIjpbIi9pbmdyZXNzIl0sInN1YiI6WyIvZWdyZXNzLyR7Y2xpZW50aWR9Il0sImFsbCI6W119fQ.XpEdW4UUjTaD0nWB9r4lvWeggmtEcuNYtfLgx1-NpRQ";
    //auto auth_token = persistence.document["auth_token"].as<ps::string>();

    mqtt_client -> begin(UNIT_UUID, auth_token.c_str());
  }
  AsyncElegantOTA.begin(&server, OTA_USERNAME, OTA_PASSWORD); // Init OTA Updates.

  ESP_LOGI("Unit", "Functions Loaded.");
  functions = load_functions();

  unit = ps::make_shared<Unit>(functions, UNIT_UUID, POWER_SENSE);
  ESP_LOGI("Unit", "Created.");

  unit -> begin(&Serial1, U1_CTRL, U1_DIR, &Serial2, U2_CTRL, U2_DIR);
  ESP_LOGI("Unit", "Started.");

  { // Create module map from discovered modules.
    auto modules = unit -> getModules();

    for (auto module : modules) {
      module_map.insert( std::make_pair( module -> getModuleID(), module ));
    }
  }

  { // Load unit data & rules from flash.
    Persistence persistence("/unit.txt", 8192, false); // Dont write anything to flash.
    auto unit_data = persistence.document.as<JsonObject>();
    unit -> load(unit_data);
  }

  { // Load module data & rules from flash
    Persistence persistence("/modules.txt", 16384, false); // Dont write anything to flash.
    auto module_data = persistence.document.as<JsonArray>();
    auto modules = unit -> getModules();

    for (auto module : modules) {
      module -> load(module_data);
    }
  }

  display -> finishLoading();
  vTaskDelay(500 / portTICK_PERIOD_MS);
  display -> showSummary();
  unit -> publish_readings = true;
  while(1) {
    xSemaphoreTake(main_task_semaphore, portMAX_DELAY);
    int64_t start_tm = millis();

    try {
      unit -> evaluateAll(); // Evaluate the unit rule engine, then evaluate the modules.
    } catch (std::exception& e) {
      ESP_LOGE("Unit", "Exception Thrown Whilst Evaluating: %s", e.what());
    } catch (...) {
      ESP_LOGE("Unit", "Something went wrong whilst evaluating.");
    }

    auto modules = unit -> getModules();
    for (auto module : modules) {
      module -> refresh();
    }

    //if (unit -> publish_readings) { // Serialize new reading when rule engine requires it.
    if (modules.at(0) -> new_readings > 10) {
      serialize_readings();
    }

    
    // Load data for the summary screen.
    if (display->pause()) {
      ESP_LOGI("Display", "Main task updating summary: %fVA, %fV %fHz %fPF %dPS.", unit -> totalApparentPower(), unit -> meanVoltage(), unit -> meanFrequency(),  unit -> meanPowerFactor(), unit -> powerStatus());
      summary_data -> mean_voltage = unit -> meanVoltage();
      summary_data -> mean_frequency = unit -> meanFrequency();
      summary_data -> mean_power_factor = unit -> meanPowerFactor();
      summary_data -> on_modules = unit -> activeModules();
      summary_data -> total_modules = unit -> moduleCount();
      summary_data -> total_apparent_power = unit -> totalApparentPower();
      summary_data -> power_status = unit -> powerStatus();
      summary_data -> connection_strength = WiFi.RSSI();
      summary_data -> nmd = 50;

      display->resume();
    }

    log_memory_usage();

    xSemaphoreGive(main_task_semaphore);

    if (millis() - start_tm > 0 && millis() - start_tm < 1000) { // Run every 1 second.
      vTaskDelay((1000 - (millis() - start_tm)) / portTICK_PERIOD_MS);
    }
    
  }

  vTaskDelete(NULL);
}


void serialize_readings() {
  try {
    auto new_message = mqtt_client -> new_outgoing_message(0, 8192); // Get object to load new message.

    new_message -> document[JSON_TYPE] = 0; // Set message type to reading.
    JsonObject data_obj = new_message -> document.createNestedObject(JSON_DATA);

    // Save period time data.
    std::pair<uint64_t, uint64_t> period = unit -> getSerializationPeriod();
    data_obj["period_start"] = period.first;
    data_obj["period_end"] = period.second;

    // Serialize modules into array.
    auto data_array = data_obj.createNestedArray(JSON_DATA);
    auto modules = unit -> getModules();
    for (auto module : modules) {
      module -> serialize(data_array); // Get each module to add its reading data.
    };

    // Message will be sent as new_message class goes out of scope now. Note: Queue insertion with portMAX_DELAY.
  } catch (...) {
    ESP_LOGE("Unit", "Failed to serialize readings.");
  }
}

void first_time_setup(Persistence& persistence) {
  persistence.clear();

  bool setup_complete = false;
  SemaphoreHandle_t setup_semaphore;
  vSemaphoreCreateBinary(setup_semaphore);
  xSemaphoreGive(setup_semaphore);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request -> send_P(200, "text/html", setup_html);
  });

  server.on("/save", HTTP_GET, [&persistence, &setup_semaphore, &setup_complete](AsyncWebServerRequest *request){
    if (request -> hasParam("ssid") && request -> hasParam("password") && request -> hasParam("setupToken")) {
      if (xSemaphoreTake(setup_semaphore, 1000 / portTICK_PERIOD_MS) == pdTRUE) {
        auto ssid = request -> getParam("ssid") -> value();
        auto pass = request -> getParam("password") -> value();
        auto token = request -> getParam("setupToken") -> value();

        ESP_LOGI("WiFi", "SSID: %s, PW: %s, JWT: %s", ssid.c_str(), pass.c_str(), token.c_str());
        persistence.document["connect_type"].set(0);
        persistence.document["ssid"].set(ssid);
        persistence.document["password"].set(pass);
        persistence.document["auth_token"].set(token);

        setup_complete = true;
        xSemaphoreGive(setup_semaphore);
        ESP_LOGI("WiFi", "Got Parameters...");
        request -> send(200, "text", "Success.");
      } else request -> send(400, "text", "Failed to take semaphore.");

    }
  });

  // Create a WiFi Access Point to allow users to input their data.
  WiFi.softAP(WIFI_SETUP_HOSTNAME, WIFI_SETUP_AP_PASSWORD);

  server.begin(); // Start the web server.

  while (1) { // Check if the user has completed the setup every 250ms.
    xSemaphoreTake(setup_semaphore, portMAX_DELAY);
    if (setup_complete) break;
    xSemaphoreGive(setup_semaphore);

    vTaskDelay(250 / portTICK_PERIOD_MS);
  }
  vTaskDelay(250 / portTICK_PERIOD_MS);
  ESP_LOGI("WiFi", "Web Setup Complete");

  server.reset(); // Clear the setup methods.

  WiFi.softAPdisconnect(true); // End the Access Point.

  persistence.enable_write(); // Write the new connection parameters to flash on destruction.

  vSemaphoreDelete(setup_semaphore); // Delete the setup semaphore as setup is now complete.
}