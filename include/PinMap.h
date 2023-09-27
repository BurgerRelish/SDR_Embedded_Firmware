#pragma once

#ifndef PIN_MAP_H
#define PIN_MAP_H

#define RS485_BAUD_RATE 115200

#define DISPLAY_SCL 1
#define DISPLAY_SDA 2

#define RESET_PIN 38
#define RESET_HOLD_LOW_TIME 5 // seconds

#define U1_RXD 18
#define U1_TXD 17
#define U1_DIR 11
#define U1_CTRL 9

#define U2_RXD 37
#define U2_TXD 36
#define U2_DIR 35
#define U2_CTRL 10

#define STATUS_LED_PIN 48

#define POWER_SENSE 4

#endif