#include <AST.h>
#include "../config.h"
#include "../DataStructures.h"
#include "../RTOSStructure.h"


void ASTTaskFunction(void*){
    ESP_LOGD("AST", "AST Task Started.");
    
    /* Infinite Loop */
    ESP_LOGI("AST", "AST Task setup complete.");
    while(1){
        vTaskDelay(10 / portTICK_PERIOD_MS);

        if(uxQueueMessagesWaiting(ASTQueue) != 0){
            auto message = new ASTQueueMessage;
            xQueueReceive(ASTQueue, message, portMAX_DELAY);

            switch(message -> action){
                case APPEND:
                break;

                case REPLACE:
                break;

                default:
                break;
            }

            delete message;
        }
    }
    
    /* If program ever breaks from the while loop, request a reinit and delete itself */
    struct AppQueueMessage message;
    message.type = ReInitAST;
    message.data = nullptr;

    xQueueSend(AppQueue, &message, 5 / portTICK_PERIOD_MS);
    vTaskDelete(NULL);
}