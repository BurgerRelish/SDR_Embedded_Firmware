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
#include "Scheduler.h"


#include "webpage_setup.h"

#include "App/Unit.h"
#include "App/Functions.h"
#include "SerializationHander.h"
#include "CommandHandler.h"

void taskApp(void* pvParameters);
void taskSentry(void* parameters);

WiFiClient wifi_client;

std::shared_ptr<re::FunctionStorage> functions; // Global rule engine function storage.
std::shared_ptr<Unit> unit; // The control unit class.
std::shared_ptr<Display> display; // The display control class.
std::shared_ptr<MQTTClient> mqtt_client; // The MQTT and (De)Compression handler class.
std::shared_ptr<Scheduler> scheduler; // The scheduler class.
std::shared_ptr<CommandHandler> command_handler; // The command handler class.
std::shared_ptr<SerializationHandler> serialization_handler; // The serialization handler class.

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

void first_time_setup(Persistence& persistence);
void save_runtime_variables();
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
    4,
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
    3,
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

  unit -> begin(&Serial1, U1_CTRL, U2_CTRL, U1_DIR);
  ESP_LOGD("App", "Unit Started.");

  scheduler = ps::make_shared<Scheduler>();
  ESP_LOGD("App", "Scheduler Created.");

  command_handler = ps::make_shared<CommandHandler>();
  command_handler -> begin(unit, scheduler);
  ESP_LOGD("App", "Command Handler Created.");

  serialization_handler = ps::make_shared<SerializationHandler>();
  serialization_handler -> begin(unit, mqtt_client);
  ESP_LOGD("App", "Serialization Handler Created.");

  { // Load the Scheduler variables from flash.
    Persistence persistence("/schedule.txt", 8192, false); // Dont write anything to flash.
    auto scheduler_data = persistence.document.as<JsonArray>();
    if (scheduler_data.size() > 0)
    scheduler -> load(scheduler_data);
  }

  { // Load the Unit variables from flash.
    Persistence persistence("/unit.txt", 8192, false); // Dont write anything to flash.
    auto unit_data = persistence.document.as<JsonObject>();
    if (unit_data.size() > 0)
    unit -> load(unit_data);
  }

  { // Load the Module variables from flash.
    Persistence persistence("/modules.txt", 16384, false); // Dont write anything to flash.
    auto module_data = persistence.document.as<JsonArray>();
    auto modules = unit -> getModules();

    if (module_data.size() > 0 && modules.size() > 0)
    for (JsonObject module : module_data) {
      ps::string target_id = module[JSON_MODULE_UID].as<ps::string>();
      auto& map = unit -> module_map;

      auto target = map.find(target_id);
      if (target == map.end()) continue; // Skip unknown.

      target -> second -> load(module); // Load into known module.
    }
  }

  display -> finishLoading();
  vTaskDelay(500 / portTICK_PERIOD_MS);
  display -> showSummary();
  uint64_t last_sample = 0;
  bool evaluate = false;
  while(1) {
    xSemaphoreTake(main_task_semaphore, portMAX_DELAY);
    int64_t start_tm = millis();

    if (millis() - last_sample > (1000 * unit -> sample_period)) { // Sample every second.
      auto modules = unit -> getModules();
      for (auto module : modules) module -> refresh();
      unit -> refresh();
    
      last_sample = millis();
      evaluate = true;
    }

    switch (unit -> mode) {
      case 0: // Rule Engine Mode
        if (evaluate) {
          try {
            unit -> evaluateAll(); // Evaluate the unit rule engine, then evaluate the modules.
          } catch (std::exception& e) {
            ESP_LOGE("Unit", "Exception Thrown Whilst Evaluating: %s", e.what());
          } catch (...) {
            ESP_LOGE("Unit", "Something went wrong whilst evaluating.");
          }
          evaluate = false;
        }
        break;
      case 1: // Scheduler Mode
        auto scheduled_changes = scheduler -> check(); // Check if any scheduled events need to be run.

        for (auto item : scheduled_changes) {
          auto module = unit -> module_map.find(item.module_id);
          if (module == unit -> module_map.end()) continue; // Skip unknown.
          module -> second -> setRelayState(item.state);
        }
        break;
    }

    serialization_handler -> serializeReadings(); // Serialize readings if it is time to do so.

    while (mqtt_client -> incoming_message_count() > 0) {
      command_handler -> handle(mqtt_client -> getMessage()); // Handle incoming messages.
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
      summary_data -> current = unit -> meanCurrent();
      display->resume();
    }

    save_runtime_variables(); // Save the runtime variables to flash if it is time to do so.

    xSemaphoreGive(main_task_semaphore);

    if (millis() - start_tm > 0 && millis() - start_tm < 1000) {
      vTaskDelay((1000 - (millis() - start_tm)) / portTICK_PERIOD_MS);
    }
    
  }
  
  ESP_LOGE("RTOS", "Main Task Escaped Loop.");
  vTaskDelete(NULL);
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
 * @brief Save all variables to flash which need to be, every 5 minutes. Saves the unit & module rule engines, as well as the scheduler.
 * 
 */
void save_runtime_variables() {
  if (!command_handler -> save_required) return;
  command_handler -> save_required = true;
  ESP_LOGI("Unit", "Saving Runtime Variables.");

  { // Save the Scheduler variables to flash.
    Persistence persistence("/sched.txt", 8192, true);
    auto scheduler_data = persistence.document.as<JsonArray>();
    scheduler -> save(scheduler_data);
  }

  { // Save the Unit variables to flash.
    Persistence persistence("/unit.txt", 8192, true);
    auto unit_data = persistence.document.as<JsonObject>();
    unit -> save(unit_data);
  }

  { // Save the Module variables to flash.
    Persistence persistence("/mod.txt", 16384, true);
    auto module_data = persistence.document.as<JsonArray>();
    auto modules = unit -> getModules();
    auto& module_map = unit -> module_map;

    ps::unordered_map<ps::string, std::shared_ptr<Module>> saved_modules;

    for (JsonObject module : module_data) { // Update existing saves.
      if (module.containsKey(JSON_MODULE_UID)) {
        auto module_id = module[JSON_MODULE_UID].as<ps::string>();
        auto found_module = module_map.find(module_id);

        if (found_module != module_map.end()) {
          found_module -> second -> save(module); // Load into known module.
          saved_modules.insert( std::make_pair( module_id, found_module -> second ));
          continue;
        } else {
          module_data.remove(module); // Remove unknown module.
        }
      }
    }

    for (auto module : modules) { // Save all modules that have not been saved.
      if (saved_modules.find(module -> getModuleID()) == saved_modules.end()) {
        JsonObject obj = module_data.createNestedObject();
        module -> save(obj);
      }
    }

  }

  ESP_LOGI("Unit", "Runtime Variables Saved.");

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
  ESP_LOGI("OTA", "Update started!");
}

void onOTAProgress(size_t current, size_t final) {
  // Log every 1 second
  if (millis() - ota_progress_millis > 1000) {
    ota_progress_millis = millis();
    ESP_LOGI("OTA", "Progress: %u%%", (current / (final / 100)));
  }
}

void onOTAEnd(bool success) {
  // Log when OTA has finished
  if (success) {
    ESP_LOGI("OTA", "Update finished successfully!");
  } else {
    ESP_LOGE("OTA", "Update failed!");
  }
}
