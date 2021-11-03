#pragma once

#include <vector>

#include <FreeRTOS.h>
#include <freertos/queue.h>
#include <sys/time.h>

#include "GetLocation.h"

namespace Watchy_Event {
typedef enum {
  NULL_EVENT = 0,
  MENU_BTN_DOWN,
  BACK_BTN_DOWN,
  UP_BTN_DOWN,
  DOWN_BTN_DOWN,
  UPDATE_SCREEN,
  LOCATION_UPDATE,
  TIME_SYNC,
  MAX
} ID;

class Event {
  public:
  ID id;
  uint64_t micros;
  union {
    Watchy_GetLocation::location loc; // LOCATION_UPDATE
    timeval tv; // TIME_SYNC
    int bounces; // BTN_DOWN
  };
  void send();
  void handle();
};

extern QueueHandle_t q;

class BackgroundTask {
 private:
  static std::vector<const BackgroundTask *> tasks;

  const char *name;
  const TaskFunction_t taskFunction;
  TaskHandle_t task;

 public:
  BackgroundTask(const char *n, const TaskFunction_t t)
      : name(n), taskFunction(t), task(nullptr){};
  const char *Name() const { return name; }
  void begin();
  void kill();
  ~BackgroundTask();

  void add() const;
  void remove() const;
  static bool running();
};

extern void setUpdateInterval(uint32_t ms);
extern void enableUpdateTimer();

extern void start();

extern void handle(); // handle all pending events
}  // namespace Watchy_Events