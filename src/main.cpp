#include <Arduino.h>
#include "sentry_task.h"
#include "SingleWire.h"
#include "SWI.h"

//SingleWire* interface_0;
#define SWI_PIN 37



void setup() {
  // put your setup code here, to run once:
//  // startSentryTask();
//   pinMode(SWI_PIN, OUTPUT);
//   auto timer = timerBegin(0, 80, true);

//   // Attach onTimer function to our timer.
//   timerAttachInterrupt(timer, &onTimer, true);

//   // Set alarm to call onTimer function every second (value in microseconds).
//   // Repeat the alarm (third parameter)
//   timerAlarmWrite(timer, 1000000, true);

//   // Start an alarm
//   timerAlarmEnable(timer);

  //interface_0 = new SingleWire(GPIO_NUM_37);
  swi_init(SWI_PIN);
}

void loop() {
  // for (uint8_t i = 0; i <= 254; i++) {
  //   if(!interface_0 -> send(i)) {
  //     ESP_LOGE("SW", "Send Failed.");
  //   }
  //   delay(50);
  // }
//   uint64_t start_tm = micros();
//  // int i = 85;
//   for (uint8_t i = 0; i <= 254; i++) {
//     if(!interface_0 -> send(i, 100000));// ESP_LOGE("SW", "Fail.");
//     interface_0 -> awaitReceive(100000);
//     int val = interface_0 -> receive();
//     if (val != i) ESP_LOGE("SW", "Invalid: %d", val);
//   }

//   uint64_t end_tm = micros();
//   float seconds = (float)(end_tm - start_tm) / 1000000;

//   ESP_LOGI("SW", "Transfer Rate: %f Bps [%fs]",(((float)(255 * 2))) / seconds, seconds);
  swi_send(85);
  delay(500);

}
