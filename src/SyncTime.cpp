#include "SyncTime.h"

#include <sntp.h>

#include "GetLocation.h"
#include "Watchy.h"
#include "WatchyErrors.h"
#include "time.h"

namespace Watchy_SyncTime {

RTC_DATA_ATTR const char *ntpServer = NTP_SERVER;
RTC_DATA_ATTR time_t lastSyncTimeTS = 0;

QueueHandle_t q;

// RTC does not know about TZ
// so DST has to be in app code
// so RTC must store UTC
// RTC computes leap year so you want year to be correct
// according to how the RTC represents it

void timeSyncCallback(struct timeval *tv) {
  Watchy_Event::Event{
    .id = Watchy_Event::TIME_SYNC,
    .micros = micros(),
    {.tv = *tv},
  }.send();
  sntp_set_sync_status(SNTP_SYNC_STATUS_COMPLETED);
  lastSyncTimeTS = tv->tv_sec;
  log_d("lastSyncTimeTS %ld", lastSyncTimeTS);
  xQueueSendToBack(q, nullptr, 0);
}

void syncTime(const char *timezone) {
  if (sntp_get_sync_status() != SNTP_SYNC_STATUS_RESET) {
    // SNTP busy
    log_i("%d", sntp_get_sync_status());
    Watchy::err = Watchy::NOT_READY;
    return;
  }
  if (!Watchy::getWiFi()) {
    log_i("getWiFi fail");
    Watchy::err = Watchy::WIFI_FAILED;
    return;
  }
  auto start = millis();
  StaticQueue_t qb;
  q = xQueueCreateStatic(1, 0, nullptr, &qb);
  sntp_set_time_sync_notification_cb(timeSyncCallback);
  configTzTime(timezone, ntpServer);
  // 30 second timeout
  if (!xQueueReceive(q, nullptr, pdMS_TO_TICKS(15000))) {
    Watchy::err = Watchy::TIMEOUT;
  }
  log_i("time sync took %ldms", millis()-start);
  Watchy::releaseWiFi();
}
}  // namespace Watchy_SyncTime