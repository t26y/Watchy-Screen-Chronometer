#include "Events.h"

#include <ctime>

#include <WiFi.h>
#include <esp_task.h>
#include <esp_task_wdt.h>
#include <freertos/queue.h>

#include "Screen.h"
#include "Watchy.h"
#include "button_interrupt.h"

namespace Watchy_Event {

RTC_DATA_ATTR uint32_t updateInterval = 0;

void setUpdateInterval(uint32_t ms) {
  updateInterval = ms;  
}

void enableUpdateTimer() {
  if (updateInterval == 0) {
    return;
  }
#ifdef ESP_RTC
  log_i("wake on esp timer %lld", uint64_t(ms) * 1000ULL);
  esp_sleep_enable_timer_wakeup(uint64_t(ms) * 1000ULL);
  updateInterval = ms;
#else
  if (updateInterval % 1000 != 0 || updateInterval >= SECS_PER_DAY * 1000) {
    log_i("esp_sleep_enable_timer_wakeup(%llu)", uint64_t(updateInterval) * 1000ULL);
    esp_sleep_enable_timer_wakeup(uint64_t(updateInterval) * 1000ULL);
    return;
  }
  time_t t = ((time(nullptr)/(updateInterval/1000))+1)*(updateInterval/1000);
  uint8_t err;
  tmElements_t tm;
  // uint32_t oldFlags = Wire.setDebugFlags(16<<24,0);
  err = Watchy::RTC.read(tm);
  if (err) {
    log_e("error reading DTC %d", err);
  }
  // Wire.setDebugFlags(oldFlags, ~oldFlags);
  log_i("     %d %02d:%02d:%02d", tm.Wday, tm.Hour, tm.Minute, tm.Second);
  log_i("%4d %d %02d:%02d:%02d", t-time(nullptr), dayOfWeek(t), numberOfHours(t), numberOfMinutes(t), numberOfSeconds(t));
  assert(tm.Wday <= 7);
  assert(tm.Hour < 24);
  assert(tm.Minute < 60);
  assert(tm.Second < 60);
  Watchy::RTC.setAlarm(ALM1_MATCH_DAY, numberOfSeconds(t), numberOfMinutes(t),
                       numberOfHours(t), dayOfWeek(t));
  Watchy::RTC.alarmInterrupt(ALARM_1, true);   // set/reset alarm interrupt
  log_i("esp_sleep_enable_ext0_wakeup(%d, %d)", RTC_PIN, 0);
  esp_sleep_enable_ext0_wakeup(RTC_PIN, 0);      // enable deep sleep wake on RTC interrupt
  log_i("stack high water %d", uxTaskGetStackHighWaterMark(nullptr));
#endif
}

const char * IDtoString(ID id) {
  switch (id) {
    case NULL_EVENT: return "NULL_EVENT";
    case MENU_BTN_DOWN: return "MENU_BTN_DOWN";
    case BACK_BTN_DOWN: return "BACK_BTN_DOWN";
    case UP_BTN_DOWN: return "UP_BTN_DOWN";
    case DOWN_BTN_DOWN: return "DOWN_BTN_DOWN";
    case UPDATE_SCREEN: return "UPDATE_SCREEN";
    case LOCATION_UPDATE: return "LOCATION_UPDATE";
    case TIME_SYNC: return "TIME_SYNC";
    case MAX: return "MAX";
    default: return "unknown";
  }
}

// we're not going to actually run as a background task, this is just to keep
// from deep sleeping while we're running.
BackgroundTask handlerTask("handler", nullptr);

void Event::handle() {
  log_i("%6d handle event %s", millis(), IDtoString(id));
  if (Watchy::screen != nullptr) {
    handlerTask.add();
    switch (id) {
      case MENU_BTN_DOWN:
        log_i("previous bounces: %d", bounces);
        Watchy::screen->menu();
        break;
      case BACK_BTN_DOWN:
        log_i("previous bounces: %d", bounces);
        Watchy::screen->back();
        break;
      case UP_BTN_DOWN:
        log_i("previous bounces: %d", bounces);
        Watchy::screen->up();
        break;
      case DOWN_BTN_DOWN:
        log_i("previous bounces: %d", bounces);
        Watchy::screen->down();
        break;
      case UPDATE_SCREEN:
        Watchy::showWatchFace(true);
        break;
      case LOCATION_UPDATE:
        Watchy_GetLocation::currentLocation = loc;
        break;
      case TIME_SYNC: 
      {
        log_i("time sync: %lu %lu", tv.tv_sec, tv.tv_usec);
        // consider using tv.tv_usec as well
        Watchy::RTC.set(tv.tv_sec);  // set RTC
        setTime(tv.tv_sec);          // set system time
        settimeofday(&tv, nullptr);    // set posix
        log_i("now %ld", now());
      }; 
      break;
      default:
        log_e("unknown event id: %d", id);
        break;
    }
    handlerTask.remove();
  }
}

void Event::send() {
  log_i("send event %s 0x%08x", IDtoString(id), this);
  xQueueSendToBack(q, reinterpret_cast<void*>(this), 0);
}

void handle() {
  Event e;
  while (xQueueReceive(q, &e, 10)) {
    e.handle();
  }
}

std::vector<const BackgroundTask*> BackgroundTask::tasks;
xSemaphoreHandle taskVectorLock = xSemaphoreCreateMutex();

void BackgroundTask::add() const {
  xSemaphoreTake(taskVectorLock, portMAX_DELAY);
  log_d("%6d %s %08x", millis(), name, reinterpret_cast<uint32_t>(this));
  tasks.push_back(this);
  xSemaphoreGive(taskVectorLock);
}

void BackgroundTask::remove() const {
  xSemaphoreTake(taskVectorLock, portMAX_DELAY);
  log_d("%6d %s %08x", millis(), Name(), reinterpret_cast<uint32_t>(this));
  for (auto it = tasks.begin(); it != tasks.end(); it++) {
    if (*it == this) {
      it = tasks.erase(it);
      break;
    }
  }
  xSemaphoreGive(taskVectorLock);
}

bool BackgroundTask::running() {
  xSemaphoreTake(taskVectorLock, portMAX_DELAY);
  bool r = tasks.size() > 0;
  xSemaphoreGive(taskVectorLock);
  return r;
}

void BackgroundTask::begin() {
  add();
  BaseType_t res = xTaskCreatePinnedToCore(
      [](void *p) {
        BackgroundTask *b = static_cast<BackgroundTask*>(p);
        b->taskFunction(nullptr);
        b->kill();
      },
      name, 4096, reinterpret_cast<void *>(this), tskIDLE_PRIORITY, &task, 1);
  configASSERT(task);
  if (res != pdPASS) {
    log_d("create background task result %d", res);
  }
}

void BackgroundTask::kill() {
  if (task != nullptr) {
    TaskHandle_t t = task;
    log_i("%s stack high water %d", Name(), uxTaskGetStackHighWaterMark(t));
    task = nullptr;
    remove();
    vTaskDelete(t); // doesn't return, because it's deleting us...
  }
}

BackgroundTask::~BackgroundTask() {
  kill();
}

QueueHandle_t q;

void start(void) {
  Watchy::initTime(); // so we can use now() in rate limits
  q = xQueueCreate(10, sizeof(Event));

  buttonSetup(MENU_BTN_PIN, menu_btn);
  buttonSetup(BACK_BTN_PIN, back_btn);
  buttonSetup(UP_BTN_PIN, up_btn);
  buttonSetup(DOWN_BTN_PIN, down_btn);
  // register for RTC gpio to send screen update events during long running
  // tasks. Figure out how to do this for ESP RTC wakeup timer too.
}
}  // namespace Watchy_Event