#pragma once

#ifndef STATUS_LED_H
#define STATUS_LED_H
#include <Arduino.h>
#include <FastLED.h>

#include "config.h"
#include "pin_map.h"

enum statusLEDState {
    STATUS_LED_SETUP,
    STATUS_LED_CONNECTING,
    STATUS_LED_DISCOVERING_MODULES,
    STATUS_LED_RUNNING,
    STATUS_LED_HIDE,
    STATUS_LED_ERROR
};

class StatusLED {

    protected:
        CRGB* status_led;
        statusLEDState status_led_state;
        uint8_t status_led_hue;

        SemaphoreHandle_t status_led_mutex;

    private:
        /**
         * The function updates the status LED with a rainbow effect.
         */
        void loopRainbow() {
            status_led_hue += deltaHue;
        }

        /**
         * @brief The `setStatic` function is used to set a static color for the status LED. It takes a hue value as a parameter and updates the `status_led_hue`
         *  variable with the new value if it is different from the current hue. This function is called in the `statusLEDStateMachine` function to set the
         *  appropriate static color based on the current state of the status LED.
         */
        void setStatic(const uint8_t hue) {
            if(hue != status_led_hue) {
                status_led_hue = hue;
            }
            return;
        }

        /**
         * @brief The `statusLEDStateMachine()` function is a state machine that determines the behavior of the status LED based on its current state (`status_led_state`).
         */
        void statusLEDStateMachine() {
            switch (status_led_state) {
                case STATUS_LED_SETUP:
                    setStatic(219); // Pink hsv(309, 91%, 97%)
                    break;

                case STATUS_LED_CONNECTING:
                    setStatic(83); // Light Green hsv(117, 91%, 97%)
                    break;

                case STATUS_LED_DISCOVERING_MODULES:
                    setStatic(189); // Purple hsv(267, 91%, 97%)
                    break;

                case STATUS_LED_RUNNING:
                    loopRainbow(); // Rainbow hsv(X, 91%, 97%)
                    break;

                case STATUS_LED_ERROR:
                    setStatic(0); // Red hsv(0, 91%, 97%)
                    break;

                default:
                    setStatic(21); // Orange hsv(29, 91%, 97%)
                    break;
            }
            return;
        }
    
    public:

        StatusLED() {
            status_led = new CRGB[1];

            FastLED.addLeds<WS2812, STATUS_LED_PIN, GRB>(status_led, 1);
            FastLED.setBrightness(BRIGHTNESS);

            status_led_state = STATUS_LED_SETUP;

            status_led_mutex = xSemaphoreCreateMutex();
            xSemaphoreGive(status_led_mutex);
            return;
        }

        ~StatusLED() {
            FastLED.clear();
            delete status_led;
            vSemaphoreDelete(status_led_mutex);
        }

        /**
         * @brief The `setStatusLEDState` function is used to update the state of the status LED. 
         * @param statusLEDState The state to be set.
         * @return true - Successfully updated the LED state.
         * @return false - Timed out taking the Semaphore.
         */       
        bool setStatusLEDState(statusLEDState new_state) {
            if(xSemaphoreTake(status_led_mutex, 100 / portTICK_PERIOD_MS) != pdTRUE) return false; // Return false if semaphore could not be taken.

            status_led_state = new_state;

            xSemaphoreGive(status_led_mutex);
            return true;
        }


        /**
         * @brief The `updateStatusLED()` function is responsible for updating the status LED based on its current state.
         * 
         * @return true - Successfully updated the LED.
         * @return false - Timed out taking the Semaphore.
         */
        bool updateStatusLED() {
            if(xSemaphoreTake(status_led_mutex, 100 / portTICK_PERIOD_MS) != pdTRUE) return false; // Return false if semaphore could not be taken.

            statusLEDStateMachine();

            status_led[0] = CHSV(status_led_hue, (status_led_state == STATUS_LED_HIDE) ? 0 : 240, 255);
            FastLED.show();

            xSemaphoreGive(status_led_mutex);
            return true;
        }
};


#endif