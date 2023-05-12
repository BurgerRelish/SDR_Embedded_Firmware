#include <Arduino.h>
#include <App.h>

void setup()
{
    auto SDRUnit = new App;
    SDRUnit -> begin();
}

void loop(){/*RTOS does not use the main loop.*/}