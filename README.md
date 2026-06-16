# ArduinoTaskScheduler - Lightweight Library

[![Arduino Library](https://www.ardu-badge.com/badge/ArduinoTaskScheduler.svg?)](https://www.ardu-badge.com/ArduinoTaskScheduler)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A lightweight, non-blocking task scheduler for Arduino projects.  
It allows you to periodically run functions with millisecond precision using a simple and minimal interface.  
Perfect for handling recurring tasks without blocking your `loop()`.

## Features

- **Millisecond Precision**: Uses `std::chrono` for accurate timing
- **Non-Blocking**: Keeps your `loop()` responsive
- **Multiple Schedulers**: Run several tasks at different intervals
- **ESP32 Hardware Timer**: `HwScheduler` for ISR-based ticks with deferred loop tasks
- **Lightweight**: Minimal code, easy to integrate
- **Simple API**: Just call `.run()` with your function

## Installation

### PlatformIO

Add to `platformio.ini`:

```ini
lib_deps =
    https://github.com/joruf/Task-Scheduler.git
```

Or use a local checkout (path relative to your project):

```ini
lib_deps =
    file://../../GitHub/task-scheduler
```

### Manual File Placement

1. Download the library files (`scheduler.h`, `scheduler.cpp`, `HwScheduler.h`, `HwScheduler.cpp`, `library.properties`)
2. Copy the **ArduinoTaskScheduler** folder to your Arduino libraries directory:
   - **Windows**: `Documents\Arduino\libraries\`
   - **Mac**: `Documents/Arduino/libraries/`
   - **Linux**: `~/Arduino/libraries/`

## Scheduler (loop-based)

Use `Scheduler` for tasks that can run from `loop()`.

### Example

```c++
#include <Arduino.h>
#include <scheduler.h>

// Create two scheduler instances
Scheduler scheduler_1(500);   // executes every 500 ms
Scheduler scheduler_2(1000);  // executes every 1000 ms

void setup() {
  Serial.begin(115200);
  while (!Serial) { ; }
  Serial.println("Scheduler test started");
}

void loop() {
  scheduler_1.run([]() { Serial.println("Task 1 executed at " + String(millis()) + " ms"); });

  scheduler_2.run([]() { Serial.println("Task 2 executed at " + String(millis()) + " ms"); });
}
```

## HwScheduler (ESP32 hardware timer)

`HwScheduler` uses an ESP32 hardware timer for a fixed base tick (default **100 ms**), independent of `loop()` load.

- **ISR handlers** (`addIsrTask`, `addHybridTask`): must be `IRAM_ATTR` and ISR-safe (counters, flags only)
- **Loop handlers** (`addLoopTask`, `addHybridTask`): deferred via `poll()` — safe for network, display, audio, etc.
- Intervals must be an **exact multiple** of the base tick (e.g. 100, 1000, 10000, 60000 ms with a 100 ms tick)

### Example

```c++
#include <Arduino.h>
#include <HwScheduler.h>

HwScheduler hwTimer(0, 100000); // timer 0, 100 ms tick
volatile uint16_t fastCounter = 0;

void IRAM_ATTR on100ms() {
  fastCounter++;
}

void setup() {
  Serial.begin(115200);

  hwTimer.begin();
  hwTimer.addIsrTask(100, on100ms);
  hwTimer.addLoopTask(1000, []() { Serial.printf("1s task, counter=%u\n", fastCounter); });
  hwTimer.addLoopTask(10000, []() { Serial.println("10s watchdog"); });
}

void loop() {
  hwTimer.poll(); // run deferred tasks
}
```

See also `examples/HwScheduler_Basic/`.
