#ifndef APP_H
#define APP_H

#include <Arduino.h>
#include <../config.h>

#include <../lib/Communication/MQTTClient.h>
/* RTOS */
void mainAppTask(void* pvParameters);

/* Device Initialization Commands */

void begin();




#endif