#pragma once

#include <Arduino.h>
#include <DS3232RTC.h>
#include <GxEPD2_BW.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <Wire.h>

#include "battery.h"
#include "BLE.h"
#include "bma.h"
#include "config.h"
#include "Events.h"

class Screen;

namespace Watchy {
extern DS3232RTC RTC;
extern GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display;
extern tmElements_t currentTime;
extern Screen *screen;
void init();
void initTime();
void deepSleep();

// components can register to be called whenever we wake up
typedef void (*OnWakeCallback)(const esp_sleep_wakeup_cause_t wakeup_reason);
extern void AddOnWakeCallback(const OnWakeCallback owc);
// no need for a Remove because they're all removed on deep sleep. Any component
// registering a callback has to do it when it gets initialized on wake...

// these two keep track of references to wifi and only close it when there
// are no more references to it
bool getWiFi();
void releaseWiFi();

void showWatchFace(bool partialRefresh, Screen *s = screen);
void setScreen(Screen *s);

// stored in RTC_DATA_ATTR
extern BMA423 sensor;
extern bool WIFI_CONFIGURED;
extern bool BLE_CONFIGURED;
};  // namespace Watchy
