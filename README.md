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

1. Download the library files (`scheduler.h`, `scheduler.cpp`, `library.properties`)
2. Copy the **ArduinoTaskScheduler** folder to your Arduino libraries directory:
   - **Windows**: `Documents\Arduino\libraries\`
   - **Mac**: `Documents/Arduino/libraries/`
   - **Linux**: `~/Arduino/libraries/`

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
