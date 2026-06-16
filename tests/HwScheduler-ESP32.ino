#include <Arduino.h>
#include <HwScheduler.h>
#include <scheduler.h>

HwScheduler hwTimer(0, 100000);
Scheduler loopScheduler(500);

volatile uint32_t isrCounter = 0;

void IRAM_ATTR on100ms() { isrCounter++; }

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("HwScheduler test started");

  hwTimer.begin();
  hwTimer.addIsrTask(100, on100ms);
  hwTimer.addLoopTask(1000, []() { Serial.printf("1s deferred, isrCounter=%lu\n", (unsigned long)isrCounter); });
}

void loop() {
  hwTimer.poll();
  loopScheduler.run([]() { Serial.printf("loop scheduler 500ms, millis=%lu\n", (unsigned long)millis()); });
}
