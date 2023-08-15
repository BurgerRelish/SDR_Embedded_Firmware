#include <Arduino.h>
#include <ps_stl.h>

#include "App.h"

std::shared_ptr<sdr::App> app;

void setup() {
  app = std::make_shared<sdr::App>();
  ESP_LOGI("SETUP", "Starting SDR Unit...");
  app->begin();
}

void loop() {}
