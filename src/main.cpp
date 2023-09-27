#include <Arduino.h>
#include <ps_stl.h>
#include <esp_brotli.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ElegantOTA.h>

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

void taskApp(void* pvParameters);
void taskComms(void* pvParameters);
void taskSentry(void* parameters);

WiFiClient wifi_client;

std::shared_ptr<re::FunctionStorage> functions;
std::shared_ptr<Unit> unit;
std::shared_ptr<Display> display;
std::shared_ptr<MQTTClient> mqtt_client;

ps::unordered_map<ps::string, std::shared_ptr<Module>> module_map;

TaskHandle_t sentry_task;
TaskHandle_t app_task;
SemaphoreHandle_t main_task_semaphore;

unsigned long ota_progress_millis = 0;

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
void handle_incoming_message();
void load_from_nvs();
void check_reset_condition();

void onOTAStart();
void onOTAProgress(size_t current, size_t final);
void onOTAEnd(bool success);

void setup() {
  psramInit();
  check_reset_condition();
  
  xTaskCreate(
    taskSentry,
    "Sentry Task",
    3 * 1024,
    NULL,
    1,
    &sentry_task
  );
}

void loop() {} // Loop is not used because of RTOS.

void taskSentry(void* parameters) {
  ESP_LOGD("RTOS", "Sentry Task Started.");

  // Init the filesystem.
  if(!LittleFS.begin(true)) ESP_LOGE("LittleFS", "Failed to mount.");
  else ESP_LOGD("Sentry", "LittleFS begun.");

  // Allocate display data and start the display.
  summary_data = (SummaryFrameData*) calloc(1, sizeof(SummaryFrameData));
  summary_data -> nmd = 200;

  display = ps::make_shared<Display>(DISPLAY_SCL, DISPLAY_SDA, VERSION);
  display -> begin(summary_data);
  ESP_LOGD("Sentry", "Display started.");

  // Set all the pin modes and default states.
  pinMode(U1_CTRL, OUTPUT);
  digitalWrite(U1_CTRL, HIGH);
  pinMode(U1_DIR, OUTPUT);
  digitalWrite(U1_DIR, HIGH);
  
  pinMode(U2_CTRL, OUTPUT);
  digitalWrite(U2_CTRL, HIGH);
  pinMode(U2_DIR, OUTPUT);
  digitalWrite(U2_DIR, HIGH);
  
  pinMode(STATUS_LED_PIN, OUTPUT);
  ESP_LOGD("Sentry", "Pin Modes set."); 

  // Init the Serial communication busses.
  Serial1.begin(RS485_BAUD_RATE, SERIAL_8N1, U1_RXD, U1_TXD);
  Serial2.begin(RS485_BAUD_RATE, SERIAL_8N1, U2_RXD, U2_TXD);
  ESP_LOGD("Sentry", "Serial interfaces started."); 
  

  // Create the RTOS semaphores.
  main_task_semaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(main_task_semaphore);
  ESP_LOGI("Sentry", "Semaphores created.");

  //AsyncElegantOTA.begin(&server, OTA_USERNAME, OTA_PASSWORD); // Init OTA Updates.

  

  { // Load WiFi parameters from flash.
    Persistence persistence("/conn.txt", 8192, false); // Dont write anything to flash.
    if (!persistence.document.containsKey("connect_type")) {
      ESP_LOGI("Connection", "Not configured. Starting setup page.");
      first_time_setup(persistence);
    }

      WiFi.setAutoReconnect(true);
      ps::string ssid = persistence.document["ssid"].as<ps::string>();
      ps::string pass = persistence.document["password"].as<ps::string>();

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
    ps::string auth_token = persistence.document["auth_token"].as<ps::string>();
    
    mqtt_client -> begin(UNIT_UUID, auth_token.c_str());
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }

  // ElegantOTA callbacks
  ElegantOTA.onStart(onOTAStart);
  ElegantOTA.onProgress(onOTAProgress);
  ElegantOTA.onEnd(onOTAEnd);
  ElegantOTA.setAuth(OTA_USERNAME, OTA_PASSWORD);

  ElegantOTA.begin(&server); // Start ElegantOTA

  server.begin();

  // Start the main task.
  xTaskCreate(
    taskApp,
    "App Task",
    48 * 1024,
    NULL,
    5,
    &app_task
  );

  while (1) {
    vTaskDelay(1000 / portTICK_PERIOD_MS);
    log_memory_usage();
  }

  ESP_LOGE("RTOS", "Sentry Task Escaped Loop.");
  vTaskDelete(NULL);
}

void taskApp(void* pvParameters) {
  ESP_LOGI("RTOS", "Main Task started.");

  functions = load_functions();
  ESP_LOGD("App", "Functions Loaded.");

  unit = ps::make_shared<Unit>(functions, UNIT_UUID, POWER_SENSE);
  ESP_LOGD("App", "Unit Created.");

  unit -> begin(&Serial1, U1_CTRL, U1_DIR, &Serial2, U2_CTRL, U2_DIR);
  ESP_LOGD("App", "Unit Started.");


  /*============================================= TEMPORARY =================================================*/

  ps::string expression_0 = "V > 200";
  ps::string command_0 = "setState(1);";
  ps::string expression_1 = "V < 200";
  ps::string command_1 = "setState(0);";

  for (auto module : unit -> getModules()) {
    module -> add_rule(0, expression_0, command_0);
    module -> add_rule(1, expression_1, command_1);
  }

  /*========================================================================================================*/

  load_from_nvs();
  ESP_LOGD("App", "Unit Started.");

  display -> finishLoading();
  vTaskDelay(500 / portTICK_PERIOD_MS);
  display -> showSummary();

  while(1) {
    xSemaphoreTake(main_task_semaphore, portMAX_DELAY);
    int64_t start_tm = millis();

    auto modules = unit -> getModules();
    for (auto module : modules) {
      module -> refresh();
    }
    unit -> refresh();
    
    try {
      unit -> evaluateAll(); // Evaluate the unit rule engine, then evaluate the modules.
    } catch (std::exception& e) {
      ESP_LOGE("Unit", "Exception Thrown Whilst Evaluating: %s", e.what());
    } catch (...) {
      ESP_LOGE("Unit", "Something went wrong whilst evaluating.");
    }

    
    //if (unit -> publish_readings) { // Serialize new reading when rule engine requires it.
    if (modules.at(0) -> new_readings > 10) {
      serialize_readings();
    }


    if (mqtt_client -> incoming_message_count() > 0) {
      handle_incoming_message();
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

      display->resume();
    }


    xSemaphoreGive(main_task_semaphore);

    if (millis() - start_tm > 0 && millis() - start_tm < 1000) { // Run every 1 second.
      vTaskDelay((1000 - (millis() - start_tm)) / portTICK_PERIOD_MS);
    }
    
  }
  ESP_LOGE("RTOS", "Main Task Escaped Loop.");
  vTaskDelete(NULL);
}


void serialize_readings() {
  try {
    auto new_message = mqtt_client -> new_outgoing_message(0, 16384); // Get object to load new message.

    new_message -> document[JSON_TYPE].set(0); // Set message type to reading.
    JsonObject data_obj = new_message -> document.createNestedObject(JSON_DATA);

    // Save period time data.
    std::pair<uint64_t, uint64_t> period = unit -> getSerializationPeriod();
    data_obj["period_start"].set(period.first);
    data_obj["period_end"].set(period.second);

    // Serialize modules into array.
    JsonArray data_array = data_obj.createNestedArray(JSON_READING_OBJECT);

    auto& modules = unit -> getModules();
    for (auto module : modules) {
      ESP_LOGD("Serialize", "Module: %s", module -> getModuleID().c_str());
      JsonObject obj = data_array.createNestedObject();
      module -> serialize(obj); // Get each module to add its reading data.
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

/**
 * @brief Loads a command message into the unit.
 * 
 * @param message 
 */
void load_command_message(std::shared_ptr<MessageDeserializer>& message) {
  auto& map = unit -> module_map;
  auto unit_data = message -> document["unit"].as<JsonObject>();

  unit -> load(unit_data);
  
  auto module_data = message -> document["modules"].as<JsonArray>();

  for (JsonObject data : module_data) {
    auto mid = data["module_id"].as<ps::string>();
    auto module = map.find(mid);
    if (module == map.end()) continue; // Skip unknown modules.

    module -> second -> load(data);
  }
}

void handle_incoming_message() {
  auto message = mqtt_client -> get_incoming_message();
  if (message.get() == nullptr) return;

  int type = message -> document["type"];

  switch (type) {
    case 0: // Command Message.
      load_command_message(message);
      return;
    default:
      return;
  }
}


/**
 * @brief Load Unit & Module stored data from SPIFFS with the Persistence class.
 * 
 */
void load_from_nvs() {
  { // Load unit data & rules from flash.
    Persistence persistence("/unit.txt", 8192, false); // Dont write anything to flash.
    auto unit_data = persistence.document.as<JsonObject>();
    unit -> load(unit_data);
  }

  { // Load module data & rules from flash
    Persistence persistence("/modules.txt", 16384, false); // Dont write anything to flash.
    auto module_data = persistence.document.as<JsonArray>();
    auto modules = unit -> getModules();

    for (JsonObject module : module_data) {
      ps::string target_id = module[JSON_MODULE_UID].as<ps::string>();
      auto& map = unit -> module_map;

      auto target = map.find(target_id);
      if (target == map.end()) continue; // Skip unknown.

      target -> second -> load(module); // Load into known module.
    }
  }
}

/**
 * @brief Checks whether the RESET_PIN is held low for at least `RESET_HOLD_LOW_TIME` seconds before formatting the Filesystem.
 */
void check_reset_condition() {
  pinMode(RESET_PIN, INPUT_PULLUP);

  vTaskDelay(1 / portTICK_PERIOD_MS);

  if (!digitalRead(RESET_PIN)) {
    uint64_t start_tm = millis();

    while (1) {
      if (digitalRead(RESET_PIN)) break; // Reset condition aborted.
      if (millis() - start_tm > (RESET_HOLD_LOW_TIME * 1000)) { // Reset pin has been held down for long enough.
        LittleFS.format();
        break;
      }
    }
  }
}

void onOTAStart() {
  // Log when OTA has started
  Serial.println("OTA update started!");
  // <Add your own code here>
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    Serial.printf("OTA Progress Current: %u bytes, Final: %u bytes\n", current, final);
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    Serial.println("OTA update finished successfully!");
  } else {
    Serial.println("There was an error during OTA update!");
  }
  // <Add your own code here>
}
