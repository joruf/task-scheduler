#include "HwScheduler.h"

#if defined(ESP32)

HwScheduler *HwScheduler::activeInstance = nullptr;

HwScheduler::HwScheduler(uint8_t timerIndex, uint32_t tickMicros)
    : timerIndex_(timerIndex), tickMicros_(tickMicros) {}

HwScheduler::~HwScheduler() { end(); }

uint32_t HwScheduler::tickMs() const {
  return tickMicros_ / 1000U;
}

uint32_t HwScheduler::msToTicks(uint32_t periodMs) const {
  const uint32_t baseMs = tickMs();
  if (baseMs == 0U || (periodMs % baseMs) != 0U) {
    return 0U;
  }
  return periodMs / baseMs;
}

bool HwScheduler::begin() {
  if (running_) {
    return true;
  }
  if (activeInstance != nullptr && activeInstance != this) {
    return false;
  }

  timer_ = timerBegin(timerIndex_, 80, true);
  if (timer_ == nullptr) {
    return false;
  }

  activeInstance = this;
  timerAttachInterrupt(timer_, &HwScheduler::onTimer, true);
  timerAlarmWrite(timer_, tickMicros_, true);
  timerAlarmEnable(timer_);
  running_ = true;
  return true;
}

void HwScheduler::end() {
  if (!running_) {
    return;
  }

  if (timer_ != nullptr) {
    timerAlarmDisable(timer_);
    timerDetachInterrupt(timer_);
    timerEnd(timer_);
    timer_ = nullptr;
  }

  if (activeInstance == this) {
    activeInstance = nullptr;
  }
  running_ = false;
}

int HwScheduler::addTask(uint32_t periodMs, IsrHandler isrHandler, LoopHandler loopHandler) {
  if (taskCount_ >= MAX_TASKS) {
    return -1;
  }
  if (isrHandler == nullptr && !loopHandler) {
    return -1;
  }

  const uint32_t periodTicks = msToTicks(periodMs);
  if (periodTicks == 0U) {
    return -1;
  }

  Task &task = tasks_[taskCount_];
  task.periodTicks = periodTicks;
  task.isrHandler = isrHandler;
  task.loopHandler = std::move(loopHandler);
  task.pending = false;
  return static_cast<int>(taskCount_++);
}

int HwScheduler::addLoopTask(uint32_t periodMs, LoopHandler handler) {
  return addTask(periodMs, nullptr, std::move(handler));
}

int HwScheduler::addIsrTask(uint32_t periodMs, IsrHandler handler) {
  return addTask(periodMs, handler, nullptr);
}

int HwScheduler::addHybridTask(uint32_t periodMs, IsrHandler isrHandler, LoopHandler loopHandler) {
  return addTask(periodMs, isrHandler, std::move(loopHandler));
}

void IRAM_ATTR HwScheduler::onTimer() {
  if (activeInstance != nullptr) {
    activeInstance->handleTimer();
  }
}

void IRAM_ATTR HwScheduler::handleTimer() {
  tickCounter_++;

  for (uint8_t i = 0; i < taskCount_; i++) {
    Task &task = tasks_[i];
    if ((tickCounter_ % task.periodTicks) != 0U) {
      continue;
    }

    if (task.isrHandler != nullptr) {
      task.isrHandler();
    }
    if (task.loopHandler) {
      task.pending = true;
    }
  }
}

void HwScheduler::poll() {
  for (uint8_t i = 0; i < taskCount_; i++) {
    Task &task = tasks_[i];
    if (!task.pending) {
      continue;
    }
    task.pending = false;
    if (task.loopHandler) {
      task.loopHandler();
    }
  }
}

#endif
