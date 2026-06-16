/**
 * @file HwScheduler.h
 * @brief ESP32 hardware-timer scheduler with ISR hooks and deferred loop tasks.
 *
 * Uses a periodic hardware timer interrupt for precise timing independent of
 * loop() load. Lightweight work may run in the ISR; heavier work is deferred
 * and executed from poll() in loop() context.
 */

#pragma once

#include <cstdint>
#include <functional>

#if defined(ESP32)

#include <esp32-hal-timer.h>

/**
 * @class HwScheduler
 * @brief Periodic task scheduler based on an ESP32 hardware timer.
 *
 * Example:
 * @code
 * HwScheduler hwTimer(0, 100000); // 100 ms base tick
 *
 * void IRAM_ATTR on100ms() { enc_inactivity++; }
 *
 * void setup() {
 *   hwTimer.begin();
 *   hwTimer.addIsrTask(100, on100ms);
 *   hwTimer.addLoopTask(1000, []() { updateClock(); });
 *   hwTimer.addLoopTask(10000, []() { checkStream(); });
 * }
 *
 * void loop() {
 *   hwTimer.poll();
 * }
 * @endcode
 */
class HwScheduler {
 public:
  static constexpr uint8_t MAX_TASKS = 8;

  using IsrHandler = void (*)();
  using LoopHandler = std::function<void()>;

  /**
   * @param timerIndex ESP32 hardware timer index (0-3).
   * @param tickMicros Base tick period in microseconds (default 100 ms).
   */
  explicit HwScheduler(uint8_t timerIndex = 0, uint32_t tickMicros = 100000);

  HwScheduler(const HwScheduler &) = delete;
  HwScheduler &operator=(const HwScheduler &) = delete;

  ~HwScheduler();

  /**
   * @brief Start the hardware timer interrupt.
   * @return true on success.
   */
  bool begin();

  /**
   * @brief Stop the timer and detach the interrupt.
   */
  void end();

  /**
   * @brief Register a task executed from poll() in loop() context.
   * @param periodMs Interval in milliseconds; must be a multiple of the base tick.
   * @param handler Callback executed from poll().
   * @return Task index (>= 0) or -1 on failure.
   */
  int addLoopTask(uint32_t periodMs, LoopHandler handler);

  /**
   * @brief Register a lightweight handler called directly from the ISR.
   * @param periodMs Interval in milliseconds; must be a multiple of the base tick.
   * @param handler ISR-safe callback (IRAM_ATTR, no blocking calls).
   * @return Task index (>= 0) or -1 on failure.
   */
  int addIsrTask(uint32_t periodMs, IsrHandler handler);

  /**
   * @brief Register an ISR hook and a deferred loop handler for the same period.
   * @param periodMs Interval in milliseconds; must be a multiple of the base tick.
   * @param isrHandler ISR-safe callback.
   * @param loopHandler Callback executed from poll().
   * @return Task index (>= 0) or -1 on failure.
   */
  int addHybridTask(uint32_t periodMs, IsrHandler isrHandler, LoopHandler loopHandler);

  /**
   * @brief Execute pending deferred loop tasks.
   *
   * Call this regularly from loop().
   */
  void poll();

  /**
   * @brief Base tick period in milliseconds.
   * @return Tick period derived from tickMicros.
   */
  uint32_t tickMs() const;

 private:
  struct Task {
    uint32_t periodTicks = 0;
    IsrHandler isrHandler = nullptr;
    LoopHandler loopHandler = nullptr;
    volatile bool pending = false;
  };

  static void IRAM_ATTR onTimer();
  void IRAM_ATTR handleTimer();

  int addTask(uint32_t periodMs, IsrHandler isrHandler, LoopHandler loopHandler);
  uint32_t msToTicks(uint32_t periodMs) const;

  static HwScheduler *activeInstance;

  uint8_t timerIndex_;
  uint32_t tickMicros_;
  hw_timer_t *timer_ = nullptr;
  Task tasks_[MAX_TASKS];
  uint8_t taskCount_ = 0;
  volatile uint32_t tickCounter_ = 0;
  bool running_ = false;
};

#endif
