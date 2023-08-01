#pragma once


#ifndef SWI_H
#define SWI_H
#include <Arduino.h>

void swi_init(uint8_t pin);
bool swi_send(uint8_t data);
int swi_receive();

#endif