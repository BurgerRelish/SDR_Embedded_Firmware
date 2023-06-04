#include <Communication.h>
#include "../config.h"
#include "../RTOSStructure.h"
#include "../DataStructures.h"


void CommsTaskFunction(void*){
    ESP_LOGD("COMMS", "Comms Task Started.");

    /* Infinite Loop */
    ESP_LOGI("COMMS", "Comms Task setup complete.");
    while(1){
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    /* If program ever breaks from the while loop, request a reinit and delete itself */
    struct AppQueueMessage message;
    message.type = ReInitComms;
    message.data = nullptr;

    xQueueSend(AppQueue, &message, 5 / portTICK_PERIOD_MS);
    vTaskDelete(NULL);
}