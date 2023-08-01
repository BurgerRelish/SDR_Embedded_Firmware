#include "SWI.h"

#define SWI_SEND_OPERATIONS 0b0000100000000000 // 0 - Continue, 1- End
#define SWI_SEND_MODES 0b0000010000000000 // 1-Read, 0 - Write

volatile uint8_t swi_pin;
hw_timer_t* swi_send_timer;

volatile uint16_t modes = 0;
volatile uint16_t operations = 0;
volatile uint16_t out_data = 0;
volatile uint8_t in_data = 0;
volatile uint8_t state = 0;
volatile uint8_t count = 0;
const uint16_t mask = 1;



void ARDUINO_ISR_ATTR swi_send_clock() {
    //count++;
    switch (state) {
        case 0: // Set START condition.
            digitalWrite(swi_pin, LOW);
            timerAlarmWrite(swi_send_timer, 6, true);
            state = 1;
            return;
            
        case 1:
            if (modes & mask) {
                pinMode(swi_pin, INPUT);
                timerAlarmWrite(swi_send_timer, 30, true);
                state = 3;
            } else {
                uint8_t level = out_data & mask;
                digitalWrite(swi_pin, level);
                out_data >>= 1;
                timerAlarmWrite(swi_send_timer, 60, true);
                state = 4;
            }
            return;

        case 3:
            in_data <<= 1;
            in_data |= digitalRead(swi_pin);
            timerAlarmWrite(swi_send_timer, 30, true);
            state = 4;
            return;
        break;
        case 4:
            pinMode(swi_pin, OUTPUT_OPEN_DRAIN);
            digitalWrite(swi_pin, HIGH);
            state = 0;
            operations >>= 1;
            modes >>= 1;
            timerAlarmWrite(swi_send_timer, 10, true);
        break;
    }

    if (operations & mask) {
        //ESP_LOGI("SWI", "Send Operation Completed: %u", count);
        pinMode(swi_pin, INPUT);
        timerAlarmDisable(swi_send_timer);
        timerStop(swi_send_timer);
    }
    return;
}

volatile uint8_t receive_data = 0;
volatile uint16_t receive_count = 0;

void ARDUINO_ISR_ATTR swi_recieve_isr() {
    cli();
    delayMicroseconds(15);
    if (receive_count == 9) {
        pinMode(swi_pin, OUTPUT_OPEN_DRAIN);
        digitalWrite(swi_pin, LOW);
        delayMicroseconds(60);
        digitalWrite(swi_pin, HIGH);
        
        receive_count = 10;
        detachInterrupt(swi_pin);
        return;
    }

    pinMode(swi_pin, INPUT);
    in_data <<= 1;
    delayMicroseconds(30);
    in_data |= digitalRead(swi_pin);

    receive_count++;
    sei();
}

void swi_init(uint8_t pin) {
    swi_pin = pin;
    receive_count = 0;
    pinMode(swi_pin, INPUT);
    attachInterrupt(digitalPinToInterrupt(swi_pin), swi_recieve_isr, FALLING);

    swi_send_timer = timerBegin(0, 80, true);
    timerAttachInterrupt(swi_send_timer, swi_send_clock, false);
}

bool swi_send(uint8_t data) {
    modes = SWI_SEND_MODES;
    operations = SWI_SEND_OPERATIONS;
    out_data = data;
    state = 0;
    count = 0;
    in_data = 255;

    detachInterrupt(digitalPinToInterrupt(swi_pin));
    timerWrite(swi_send_timer, 0);
    timerAlarmWrite(swi_send_timer, 10, true);
    timerStart(swi_send_timer);
    timerAlarmEnable(swi_send_timer);
    while(timerStarted(swi_send_timer)) {}

    attachInterrupt(digitalPinToInterrupt(swi_pin), swi_recieve_isr, FALLING);

    if (in_data != 254) return false; // No ACK from Receiver.

    return true;
}

int swi_receive() {
    if (receive_count != 10) return -1;

    uint8_t data = receive_data;
    receive_data = 0;
    receive_count = 0;
    attachInterrupt(digitalPinToInterrupt(swi_pin), swi_recieve_isr, FALLING);

    return data;
}