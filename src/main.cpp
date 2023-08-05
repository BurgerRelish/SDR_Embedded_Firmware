#include <Arduino.h>
#include "../../lib/SDRApp.h"

using namespace SDR;

AppClass app;

void setup() {
  app = AppClass();
  app.begin();
}

void loop() {}
