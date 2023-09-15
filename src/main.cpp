#include <Arduino.h>
#include <ps_stl.h>
#include <esp_brotli.h>
#include <WiFi.h>

//#include "App.h"

#include "config.h"
#include "pin_map.h"
#include "Unit.h"
#include "Display.h"
#include "MQTTClient.h"
#include "MessageSerializer.h"
#include "MessageDeserializer.h"

#include "functions/functions.h"

//std::shared_ptr<sdr::App> app;
void taskMain(void* pvParameters);
void taskComms(void* pvParameters);

std::shared_ptr<re::FunctionStorage> functions;
std::shared_ptr<Unit> unit;
Display display(DISPLAY_SCL, DISPLAY_SDA);

SemaphoreHandle_t summary_semaphore;
SummaryFrameData summary_data;

std::shared_ptr<MQTTClient> mqtt_client;

void setup() {
  pinMode(U1_CTRL, OUTPUT);
  digitalWrite(U1_CTRL, HIGH);
  pinMode(U1_DIR, OUTPUT);
  digitalWrite(U1_DIR, HIGH);
  
  pinMode(U2_CTRL, OUTPUT);
  digitalWrite(U2_CTRL, HIGH);
  pinMode(U2_DIR, OUTPUT);
  digitalWrite(U2_DIR, HIGH);
  
  pinMode(STATUS_LED_PIN, OUTPUT);


  memset(&summary_data, 0, sizeof(SummaryFrameData)); // Set everything in the summary frame to zero.
  display.begin(&summary_data);
  display.startLoading();

  Serial1.begin(RS485_BAUD_RATE, SERIAL_8N1, U1_RXD, U1_TXD);
  Serial2.begin(RS485_BAUD_RATE, SERIAL_8N1, U2_RXD, U2_TXD);


}

void loop() {}

TaskHandle_t main_task;
TaskHandle_t comms_task;
SemaphoreHandle_t main_task_semaphore;
SemaphoreHandle_t comms_task_semaphore;

void taskMain(void* pvParameters) {
  unit = ps::make_shared<Unit>(functions, UNIT_UUID, POWER_SENSE);
  functions = load_functions();

  unit -> begin(&Serial1, U1_CTRL, U1_DIR, &Serial2, U2_CTRL, U2_DIR);

  xTaskCreate(
    taskComms,
    "Communications Task",
    COMMS_TASK_STACK,
    NULL,
    COMMS_PRIORITY,
    &comms_task
  );

  vTaskDelay(5 / portTICK_PERIOD_MS);
  xSemaphoreTake(comms_task_semaphore, portMAX_DELAY);
  xSemaphoreGive(comms_task_semaphore);

  while(1) {
    xSemaphoreTake(main_task_semaphore, portMAX_DELAY);
    xSemaphoreGive(main_task_semaphore);

    uint64_t start_tm = millis();

    unit -> refresh();
    unit -> evaluateAll();

    xSemaphoreTake(summary_semaphore, portMAX_DELAY);
    summary_data.mean_voltage = unit -> meanVoltage();
    summary_data.mean_frequency = unit -> meanFrequency();
    summary_data.mean_power_factor = unit -> meanPowerFactor();
    summary_data.on_modules = unit -> activeModules();
    summary_data.total_modules = unit -> moduleCount();
    summary_data.total_apparent_power = unit -> totalApparentPower();
    xSemaphoreGive(summary_semaphore);

    vTaskDelay((5000 - (millis() - start_tm)) / portTICK_PERIOD_MS);
  }

  vTaskDelete(NULL);
}

void taskComms(void* pvParameters) {
  WiFi.begin("Routy", "0609660733");


  display.finishLoading();
  while(1) {
    xSemaphoreTake(summary_semaphore, portMAX_DELAY);
    summary_data.connection_strength = WiFi.RSSI();
    summary_data.nmd = 13284;
    xSemaphoreGive(summary_semaphore);
    MessageDeserializer deserializer("test123");
    MessageSerializer serializer(mqtt_client, 1, 8192);
    vTaskDelay(1 / portTICK_PERIOD_MS);
  }

  vTaskDelete(NULL);
}