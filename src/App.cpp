#include <App.h>

#include "../DataStructures.h"
#include "../RTOSStructure.h"

#include "AST.h"
#include "Communication.h"
#include "Control.h"

#ifdef USE_GSM
    #include <HardwareSerial.h>

    #define TINY_GSM_MODEM_SARAR4
    #define SerialAT Serial1

    #include <TinyGsmClient.h>

    TinyGsm modem(SerialAT);
    TinyGsmClient client(modem);
#else
    #include <WiFi.h>
    WiFiClient client;
#endif

/* RTOS */
void AppTaskFunction(void* pvParameters);

/* Function Definitions */
void initSemaphores();
void initQueues();
void initAppTask();
void initControlTask();
void initCommsTask();
void initASTTask();

void scanModules(struct ModuleMetaData* output);

void processReadingPacket(struct ReadingPacket* packet);
void processCommandPacket(struct CommandPacket* packet);

void begin()
{
    // Create Semaphores and Mutexes
    initSemaphores();

    // Create Queues
    initQueues();

    // Create the main App Task
    initAppTask();

    ESP_LOGI("APP", "Setup Complete.");

    return;
}

void AppTaskFunction(void* pvParameters)
{
    ESP_LOGD("APP", "Main App Task started successfully.");
    /* Setup */

    // Start Comms Task
    initCommsTask();

    // Start AST Task
    initASTTask();

    // Read NVS Data
    ESP_LOGD("APP", "Reading NVS.");

    // Scan for Connected Modules.
    auto module_metadata_tree = (struct ModuleMetaData*) ps_malloc(sizeof(struct ModuleMetaData)); // Start the Module Metadata tree on PSRAM
    scanModules(module_metadata_tree);

    // Verify Module IDs with NVS
    ESP_LOGD("APP", "Verifying Module IDs.");
        //Send request to communications task to update server of new/missing devices

    // Write Meter Calibration Parameters
    ESP_LOGD("APP", "Writing Meter Calibration Parameters.");
    // Start Control Task
    initControlTask();

    struct ASTNode* AST_start_node = nullptr;

    /* Infinite Loop */
    ESP_LOGI("APP", "Main App Task setup complete.");
    while(1){
        vTaskDelay(10 / portTICK_PERIOD_MS);

        if(uxQueueMessagesWaiting(AppQueue) != 0){
            auto message = new AppQueueMessage;
            xQueueReceive(AppQueue, message, portMAX_DELAY);

            switch(message -> type){
                case ReadingPacket:
                    processReadingPacket((struct ReadingPacket*) message -> data);
                break;

                case CommandPacket:
                    processCommandPacket((struct CommandPacket*) message -> data);
                break;

                case ASTNode:
                    AST_start_node = (struct ASTNode*) message -> data;
                break;

                case ReInitAST:
                    initASTTask();
                break;

                case ReInitControl:
                    initControlTask();
                break;

                case ReInitComms:
                    initCommsTask();
                break;

                default:
                ESP_LOGE("APP", "Got Unknown Packet. Type: %d", message -> type);
                break;
            }

            delete message;
        }
    }
    ESP_LOGE("APP", "App task escaped while loop. Rebooting...");
    esp_restart();
    vTaskDelete(NULL);
}

/* Functions */
void initSemaphores(){
    ESP_LOGD("APP", "Creating Mutexes and Semaphores.");

    // Create Mutexes and Semaphores
    ASTMutex = xSemaphoreCreateMutex();

    // Make Mutexes/Semaphores available
    xSemaphoreGive(ASTMutex);

    return;
}

void initQueues(){
    ESP_LOGD("APP", "Creating Queues.");

    AppQueue = xQueueCreate(APP_QUEUE_SIZE, sizeof(AppQueueMessage));
    ASTQueue = xQueueCreate(AST_QUEUE_SIZE, sizeof(ASTQueueMessage));
    ControlQueue = xQueueCreate(CONTROL_QUEUE_SIZE, sizeof(ControlQueueMessage));
    CommsQueue = xQueueCreate(COMMS_QUEUE_SIZE, sizeof(CommsQueueMessage));

    return;
}

void initAppTask(){
    ESP_LOGD("APP", "Creating App Task.");

    xTaskCreate( // Create the main app task.
        AppTaskFunction,
        "Main App Task",
        APP_TASK_STACK,
        NULL,
        APP_PRIORITY,
        &AppTask
    );

    return;
}

void initControlTask(){
    ESP_LOGD("APP", "Starting Control Task.");

    xTaskCreate(
        ControlTaskFunction,
        "Control Task",
        CONTROL_TASK_STACK,
        NULL,
        CONTROL_PRIORITY,
        &ControlTask
    );

    return;
}

void initCommsTask(){
    ESP_LOGD("APP", "Starting Comms Task.");

    xTaskCreate(
        CommsTaskFunction,
        "Communication Task",
        COMMS_TASK_STACK,
        NULL,
        COMMS_PRIORITY,
        &CommsTask
    );

    return;
}

void initASTTask(){
    ESP_LOGD("APP", "Starting AST Task.");

    xTaskCreate(
        ASTTaskFunction,
        "AST Manager Task",
        AST_TASK_STACK,
        NULL,
        AST_PRIORITY,
        &ASTTask
    );

    return;
}

void scanModules(struct ModuleMetaData* output){
    // Scan I2C Bus and Store Addresses to PSRAM
    ESP_LOGD("APP", "Scanning I2C Bus.");

    // Scan SPI for EEPROM addresses
    ESP_LOGD("APP", "Scanning for EEPROMs.");
    // Read Module IDs from EEPROMs
    ESP_LOGD("APP", "Reading Module IDs.");

}

void processReadingPacket(struct ReadingPacket* packet){
    struct ReadingStorePacket* current_packet = packet -> module -> storage;

    // Traverse to the end of the linked list
    while(current_packet -> next_reading != nullptr){
        current_packet = current_packet -> next_reading;
    }

    // Create a new storage packet
    auto new_packet = (struct ReadingStorePacket*) ps_malloc(sizeof(struct ReadingStorePacket));

    // Append packet to the end of the linked list
    current_packet -> next_reading = new_packet;
    new_packet -> next_reading = nullptr;

    // Copy packet data to PSRAM
    new_packet -> active_power = packet -> active_power;
    new_packet -> reactive_power = packet -> reactive_power;
    new_packet -> apparent_power = packet -> apparent_power;
    new_packet -> power_factor = packet -> power_factor;
    new_packet -> voltage = packet -> voltage;
    new_packet -> frequency = packet -> frequency;
    new_packet -> timestamp = packet -> timestamp;
    new_packet -> kwh_usage = packet -> kwh_usage;

    // Free packet on SRAM
    free(packet);

    return;
}

void processCommandPacket(struct CommandPacket* packet){
    switch(packet -> action){
        case DIRECT_ON:
        break;
        case DIRECT_OFF:
        break;
        case SCHEDULE_ON:
        break;
        case SCHEDULE_OFF:
        break;
        case STATUS_REQUEST:
        break;
        case PUBLISH_REQUEST:
        break;
        case EXEMPT:
        break;
        
        default:
            ESP_LOGE("APP", "Got Unknown Command Packet. Type: %d", packet -> action);
        break;
    }
}