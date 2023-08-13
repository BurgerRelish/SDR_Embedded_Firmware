#include <Arduino.h>
#include <memory>
#include <WiFi.h>
#include "../app/App.h"
#include "../sdr/Module.h"
#include "../sdr/Unit.h"
#include <ps_stl.h>
std::shared_ptr<sdr::App> app;

void setup() {
  app = std::make_shared<sdr::App>();
  ESP_LOGI("SETUP", "Starting SDR Unit...");
  app->begin();
}

void loop() {}
