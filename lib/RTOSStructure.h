#ifndef RTOS_STRUCTURE_H
#define RTOS_STRUCTURE_H

#include <Arduino.h>

/* RTOS Config */
// Main App
constexpr uint32_t APP_TASK_STACK = 32768;
constexpr uint8_t APP_QUEUE_SIZE = 10;
constexpr uint8_t APP_PRIORITY = 1;

// Control Task
constexpr uint32_t CONTROL_TASK_STACK = 32768;
constexpr uint8_t CONTROL_QUEUE_SIZE = 10;
constexpr uint8_t CONTROL_PRIORITY = 2;

// AST Manager Task
constexpr uint32_t AST_TASK_STACK = 32768;
constexpr uint8_t AST_QUEUE_SIZE = 10;
constexpr uint8_t AST_PRIORITY = 3;

// Comms Task
constexpr uint32_t COMMS_TASK_STACK = 32768;
constexpr uint8_t COMMS_QUEUE_SIZE = 10;
constexpr uint8_t COMMS_PRIORITY = 4;


/* Semaphores */

/* Mutexes */
xSemaphoreHandle ASTMutex = NULL;

/* Queues */
xQueueHandle AppQueue = NULL;
xQueueHandle ASTQueue = NULL;
xQueueHandle CommsQueue = NULL;
xQueueHandle ControlQueue = NULL;

/* Task Handles */
xTaskHandle AppTask = NULL;
xTaskHandle ControlTask = NULL;
xTaskHandle CommsTask = NULL;
xTaskHandle ASTTask = NULL;

#endif