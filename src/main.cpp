#include <Arduino.h>
#include <ps_stl.h>
#include <esp_brotli.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <time.h>


#include "Config.h"
#include "PinMap.h"
#include "Display.h"
#include "MQTTClient.h"

#include "App/Unit.h"
#include "App/Functions.h"

void taskMain(void* pvParameters);
void taskComms(void* pvParameters);

WiFiClient wifi_client;

std::shared_ptr<re::FunctionStorage> functions;
std::shared_ptr<Unit> unit;
std::shared_ptr<Display> display;
std::shared_ptr<MQTTClient> mqtt_client;

TaskHandle_t main_task;
SemaphoreHandle_t main_task_semaphore;

SummaryFrameData* summary_data;

// AsyncWebServer server(80);

void log_memory_usage() {
    static const size_t tot_psram = heap_caps_get_total_size(MALLOC_CAP_SPIRAM);
    static const size_t tot_sram = heap_caps_get_total_size(MALLOC_CAP_INTERNAL);

    size_t free_psram = heap_caps_get_free_size(MALLOC_CAP_SPIRAM);
    size_t free_sram = heap_caps_get_free_size(MALLOC_CAP_INTERNAL);

    log_printf("- PSRAM Usage: %.4f/%u KB (%f%%)\n", ((float)(tot_psram - free_psram)) / 1024, tot_psram / 1024, 100 *( 1 - ((float) free_psram) / ((float) tot_psram)));
    log_printf("- SRAM Usage: %.4f/%u KB (%f%%)\n", ((float)(tot_sram - free_sram)) / 1024, tot_sram / 1024, 100 *( 1 - ((float) free_sram) / ((float) tot_sram)));
}


void setup() {
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
  sizeof(Reading);
  sizeof(Module);
  sizeof(Unit);
  
  ESP_LOGI("PINS", "Modes set."); 

  main_task_semaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(main_task_semaphore);

  ESP_LOGI("RTOS", "Semaphores created.");

  delay(100);

  // ESP_LOGI("Pins", "Serial ports started.");
  // AsyncElegantOTA.begin(&server);
  // server.begin();

  xTaskCreate(
    taskMain,
    "Main Task",
    32 * 1024,
    NULL,
    5,
    &main_task
  );
  log_memory_usage();
}

void loop() {
}

void taskMain(void* pvParameters) {
  ESP_LOGI("RTOS", "Main Task started.");
  WiFi.setAutoReconnect(true);
  WiFi.begin("Routy", "0609660733");

  while (WiFi.status() != WL_CONNECTED) {
    log_memory_usage();
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }

  configTime(GMT_OFFSET, 0, NTP_SERVER);

  ESP_LOGI("UNIT", "Starting MQTT Client.");
  mqtt_client = ps::make_shared<MQTTClient>(wifi_client);
  mqtt_client -> begin(CLIENT_ID, MQTT_ACCESS_TOKEN, BROKER_URL, BROKER_PORT);

  ESP_LOGI("Unit", "Functions Loaded.");
  functions = load_functions();

  unit = ps::make_shared<Unit>(functions, UNIT_UUID, POWER_SENSE);
  ESP_LOGI("Unit", "Created.");

  vTaskDelay(500 / portTICK_PERIOD_MS);

  unit -> begin(&Serial1, U1_CTRL, U1_DIR, &Serial2, U2_CTRL, U2_DIR);
  ESP_LOGI("Unit", "Started.");

  display -> finishLoading();
  display -> showSummary();

  if (unit -> getModules().size() > 0) {
    auto modules = unit -> getModules();
    ps::string expression_0 = "(TS - LEX) >= 5 && upd == 0"; // Update the module every 5 seconds.
    ps::string command_0 = "readModule();setVar(\"bool\", \"upd\", 1);";

    ps::string expression_1 = "(V > 220 && FR > 47.5) && (upd == 1)"; // Turn on if valid voltage and frequency
    ps::string command_1 = "setState(1);setVar(\"bool\", \"upd\", 0);";

    ps::string expression_2 = "(V < 220 || FR < 47.5 || FR > 52.5) && (upd == 1)"; // Turn off if out of range.
    ps::string command_2 = "setState(0);setVar(\"bool\", \"upd\", 0);";

    modules.at(0) -> add_rule(3, expression_0, command_0); // Read module first.
    modules.at(0) -> add_rule(1, expression_1, command_1);
    modules.at(0) -> add_rule(2, expression_2, command_2);
  }

  while(1) {
    xSemaphoreTake(main_task_semaphore, portMAX_DELAY);
    uint64_t start_tm = millis();

    unit -> refresh();
    // ESP_LOGI("Unit", "Refreshed.");

    //unit -> evaluateAll();
    //ESP_LOGI("Unit", "Evaluated.");
    try {
      unit -> evaluateModules();
    } catch (...) {
      ESP_LOGE("Unit", "Something went wrong whilst evaluating.");
    }
    
    try {
      auto new_message = mqtt_client -> new_outgoing_message(0, 8192); // Get new message to 
      new_message -> document["type"] = "reading";

      DynamicPSRAMJsonDocument doc(8192);

      // Load period time data.
      auto period = unit -> getSerializationPeriod();
      doc["period_start"] = std::get<0>(period);
      doc["period_end"] = std::get<1>(period);

      // Serialize modules into temp doc.
      auto data_array = doc.createNestedArray("data");
      
      auto modules = unit -> getModules();
      for (auto module: modules) {
        module -> serialize(data_array);
      }

      // Serialize module data into string
      ps::ostringstream data_field;
      auto result = serializeJson(doc, data_field);

      if (result == 0) ESP_LOGE("Serialize", "No data serialized.");

      // Set string into message format
      new_message -> document["data"] = data_field.str();

      // Message sent as new_message class goes out of scope.
    } catch (...) {
      ESP_LOGE("Unit", "Failed to serialize readings.");
    }

    if (display->pause()) {
      ESP_LOGI("Display", "Main task updating summary: %fVA, %fV %fHz %fPF %dPS.", unit -> totalApparentPower(), unit -> meanVoltage(), unit -> meanFrequency(),  unit -> meanPowerFactor(), unit -> powerStatus());
      summary_data->mean_voltage = unit -> meanVoltage();
      summary_data->mean_frequency = unit -> meanFrequency();
      summary_data->mean_power_factor = unit -> meanPowerFactor();
      summary_data->on_modules = unit -> activeModules();
      summary_data->total_modules = unit -> moduleCount();
      summary_data->total_apparent_power = unit -> totalApparentPower();
      summary_data->power_status = unit -> powerStatus();
      summary_data->connection_strength = WiFi.RSSI();
      summary_data->nmd = 50;
      display->resume();
    }

    log_memory_usage();

    // auto modules = unit -> getModules();
    // if (modules.size() > 0)
    //   modules.at(0) -> setRelayState(!(modules.at(0) -> getRelayState()));

    xSemaphoreGive(main_task_semaphore);

    vTaskDelay((1000 - (millis() - start_tm)) / portTICK_PERIOD_MS);
  }

  vTaskDelete(NULL);
}
