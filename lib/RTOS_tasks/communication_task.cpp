#include "communication_task.h"
#include "config.h"
#include "VariableDelay.h"

#include <WiFi.h>
#include "../Communication/MessageSerializer.h"
#include "../Communication/MessageDeserializer.h"
#include "../Communication/MQTTClient.h"

MQTTClient* mqtt_client;
MessageSerializer* serializer;

#define TARGET_LOOP_FREQUENCY 1
void commsTaskFunction(void* pvParameters);
void handleCommsMessage();

bool startCommsTask() {
    if (xTaskCreate(
        commsTaskFunction,
        COMMS_TASK_NAME,
        COMMS_TASK_STACK,
        NULL,
        COMMS_PRIORITY,
        &CommsTask
    ) == pdTRUE) return true;
    ESP_LOGE(COMMS_TASK_NAME, "Failed to start Comms task.");
    return false;
}   

void commsTaskFunction(void* pvParameters) {
    VariableDelay vd(RULE_ENGINE_TASK_NAME, TARGET_LOOP_FREQUENCY); // Create a new variable delay class to set target frequency.

    vd.addCallback(handleCommsMessage, 50);
    while(1) {

        vd.delay(); // Wait for remainder of frame to reach target frequency.
    }
}


void handleCommsMessage() {

}

