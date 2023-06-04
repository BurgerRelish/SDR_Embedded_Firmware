#include <Control.h>

#include "../config.h"
#include "../RTOSStructure.h"
#include "../DataStructures.h"


void ControlTaskFunction(void*){
    ESP_LOGD("CONTROL", "Control Task Started.");

    /* Infinite Loop */
    ESP_LOGI("CONTROL", "Control Task setup complete.");
    while(1){
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    /* If program ever breaks from the while loop, request a reinit and delete itself */
    struct AppQueueMessage message;
    message.type = ReInitControl;
    message.data = nullptr;

    xQueueSend(AppQueue, &message, 5 / portTICK_PERIOD_MS);
    vTaskDelete(NULL);
}