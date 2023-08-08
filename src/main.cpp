#include <Arduino.h>
#include <memory>
#include "SDRApp.h"
#include <LittleFS.h>

std::shared_ptr<SDR::AppClass> app;

void setup() {
  app = std::make_shared<SDR::AppClass>();
  ESP_LOGI("SETUP", "Starting SDR Unit...");
  app->begin();
}

void loop() {}
