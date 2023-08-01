#include "SingleWire.h"

static void swi_delay_us(uint64_t us)
{
    delayMicroseconds(us);
}

inline void swi_set_level(uint8_t pin, uint8_t level) {
    digitalWrite(pin, level);
}

inline void swi_set_direction(uint8_t pin, uint8_t mode) {
    pinMode(pin, mode);
}

inline int swi_get_level(uint8_t pin) {
    return digitalRead(pin);
}

inline uint64_t swi_micros() {
    return esp_timer_get_time();
}

void ARDUINO_ISR_ATTR rcvISR(void* parent) {
    ((SingleWire*) parent) -> receiveRequested();
}

SingleWire::SingleWire(gpio_num_t interface_pin) : pin(interface_pin), wait_receive(false) {
    attachISR();
} 

SingleWire::SingleWire(uint8_t interface_pin) : wait_receive(false) {
    pin = static_cast<gpio_num_t>(interface_pin);
    attachISR();
} 

bool SingleWire::send(uint8_t val, uint32_t timeout_us) {
    if (wait_receive) return false; // Dont send when there is a receive pending.
    detachISR();

    portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;
    portENTER_CRITICAL(&mux);

    swi_set_direction(pin, OUTPUT_OPEN_DRAIN);

    /* Write START Condition */
    swi_set_level(pin, LOW);
    swi_delay_us(PERIOD_US);
    swi_set_level(pin, HIGH);
    swi_set_direction(pin, INPUT);

    /* Wait for Slave RDY or TIMEOUT*/
    uint64_t end_tm = swi_micros() + timeout_us;

    while(swi_get_level(pin)) {
        if (end_tm < swi_micros()) {
            portEXIT_CRITICAL(&mux);
            return false;
        }
    }

    /* Wait for Slave to release line */
    swi_delay_us(PERIOD_US - 15);

    swi_set_direction(pin, OUTPUT_OPEN_DRAIN);
    for (uint8_t mask = 1; mask; mask <<= 1) { // Write 8 bits of data.
        swi_set_level(pin, (val & mask));
        swi_delay_us(PERIOD_US);
    }

    swi_set_level(pin, HIGH);
    swi_set_direction(pin, INPUT); // Release line to slave.

    /* Read Slave ACK */
    swi_delay_us(PERIOD_US / 2);
    auto retval = swi_get_level(pin);
    swi_delay_us(PERIOD_US / 2);
    swi_set_direction(pin, OUTPUT_OPEN_DRAIN);

    /* Write Master ACK */
    swi_set_level(pin, LOW);
    swi_delay_us(PERIOD_US);
    swi_set_level(pin, HIGH);
    swi_set_direction(pin, INPUT);

    portEXIT_CRITICAL(&mux);
    attachISR();
    return !retval;
}

int SingleWire::receive() {
    if (!wait_receive) return -1;
    detachISR();
    cli();

    uint8_t buffer = 0;
    swi_set_direction(pin, INPUT);
    while(!swi_get_level(pin)); // Wait for master to release interface.

    /* Write RDY condition. */
    swi_set_direction(pin, OUTPUT_OPEN_DRAIN);
    swi_set_level(pin, LOW);
    swi_delay_us(PERIOD_US);
    swi_set_level(pin, HIGH);
    swi_set_direction(pin, INPUT);

    /* Recieve 8 bits */
    swi_delay_us(PERIOD_US / 2);
    for (uint8_t mask = 1; mask; mask <<= 1) {
        if (swi_get_level(pin)) buffer |= mask;

        if ((mask << 1)) swi_delay_us(PERIOD_US); // Delay half a period on the last bit.
        else swi_delay_us(PERIOD_US / 2);
    }

    /* Write Slave ACK Condition */
    swi_set_direction(pin, OUTPUT_OPEN_DRAIN);
    swi_set_level(pin, LOW);
    swi_delay_us(PERIOD_US);
    swi_set_level(pin, HIGH);
    swi_set_direction(pin, INPUT);

    /* Read Master ACK */
    swi_delay_us(PERIOD_US / 2);
    int retval = (swi_get_level(pin)) ? -1 : (int) buffer;
    swi_delay_us(PERIOD_US / 2);

    sei();
    wait_receive = false;
    attachISR();
    return (int) retval;
}

bool SingleWire::awaitReceive(uint32_t timeout_us) {
    uint64_t end_tm = swi_micros() + timeout_us;

    while(!wait_receive) {
        if (swi_micros() > end_tm) return false;
    }

    return true;
}

void SingleWire::attachISR(){
    pinMode(pin, INPUT);
    attachInterruptArg(digitalPinToInterrupt(pin), rcvISR, this, FALLING);
}

void ARDUINO_ISR_ATTR SingleWire::detachISR(){
    detachInterrupt(digitalPinToInterrupt(pin));
}

void ARDUINO_ISR_ATTR SingleWire::receiveRequested() {
    wait_receive = true;
    detachISR();
}

