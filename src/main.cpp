#include <Arduino.h>
#include <memory>
#include "SDRApp.h"

std::shared_ptr<SDR::AppClass> app;

void setup() {
  app = std::make_shared<SDR::AppClass>();
  app -> begin();
}

void loop() {}
