#include <App.h>
#include "DataStructures.h"

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
TaskHandle_t AppHandle;

void begin()
{
    // Create Semaphores and Mutexes
    
    // Create Queues

    xTaskCreate( // Create the main app task.
        mainAppTask,
        "Main App Task",
        32768,
        NULL,
        2,
        &AppHandle
    );

    return;
}

void mainAppTask(void* pvParameters)
{
    /* Setup */

    // Start Comms Task

    // Start AST Task

    // Read NVS Data

    // Scan I2C Bus and Store Addresses to PSRAM

    // Scan SPI for EEPROM addresses

    // Read Module IDs from EEPROMs

    // Verify Module IDs with NVS
        //Send request to communications task to update server of new/missing devices

    // Write Meter Calibration Parameters

    // Start Control Task

    /* Infinite Loop */
    while(1){

    }
}