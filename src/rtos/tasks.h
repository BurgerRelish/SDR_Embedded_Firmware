#pragma once

#ifndef RTOS_TASKS_H
#define RTOS_TASKS_H

void sentryTaskFunction(void* pvParameters);
void commsTaskFunction(void* global_class);
void ruleEngineTaskFunction(void* global_class);
void controlTaskFunction(void* global_class);

#endif