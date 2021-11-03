#include "Watchy.h"

#include <vector>

#include "Events.h"
#include "GetLocation.h"  // bad dependency
#include "Screen.h"
#include "Sensor.h"
#include "WatchyErrors.h"

namespace Watchy {

Error err;

void _rtcConfig();
uint16_t _readRegister(uint8_t address, uint8_t reg, uint8_t *data,
                       uint16_t len);
uint16_t _writeRegister(uint8_t address, uint8_t reg, uint8_t *data,
                        uint16_t len);

DS3232RTC RTC(false);
GxEPD2_BW<GxEPD2_154_D67, GxEPD2_154_D67::HEIGHT> display(
  GxEPD2_154_D67(CS, DC, RESET, BUSY));
RTC_DATA_ATTR Screen *screen = nullptr;

RTC_DATA_ATTR BMA423 sensor;
RTC_DATA_ATTR bool WIFI_CONFIGURED;
RTC_DATA_ATTR bool BLE_CONFIGURED;

String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void handleButtonPress() {
  uint64_t wakeupBit = esp_sleep_get_ext1_wakeup_status();
  switch (wakeupBit & BTN_PIN_MASK) {
    case MENU_BTN_MASK:
      Watchy_Event::Event{
          .id = Watchy_Event::MENU_BTN_DOWN,
          .micros = micros(),
      }.send();
      break;
    case BACK_BTN_MASK:
      Watchy_Event::Event{
          .id = Watchy_Event::BACK_BTN_DOWN,
          .micros = micros(),
      }.send();
      break;
    case UP_BTN_MASK:
      Watchy_Event::Event{
          .id = Watchy_Event::UP_BTN_DOWN,
          .micros = micros(),
      }.send();
      break;
    case DOWN_BTN_MASK:
      Watchy_Event::Event{
          .id = Watchy_Event::DOWN_BTN_DOWN,
          .micros = micros(),
      }.send();
      break;
    default:
      break;
  }
}

QueueHandle_t i2cMutex = xSemaphoreCreateRecursiveMutex();

tmElements_t currentTime;  // should probably be in SyncTime

// doesn't persist over deep sleep. don't care.
std::vector<OnWakeCallback> owcVec;

void AddOnWakeCallback(const OnWakeCallback owc) { owcVec.push_back(owc); }

const char *wakeupReasonToString(esp_sleep_wakeup_cause_t wakeup_reason) {
  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_UNDEFINED:
      return ("UNDEFINED");
    case ESP_SLEEP_WAKEUP_ALL:
      return ("ALL");
    case ESP_SLEEP_WAKEUP_EXT0:
      return ("EXT0");
    case ESP_SLEEP_WAKEUP_EXT1:
      return ("EXT1");
    case ESP_SLEEP_WAKEUP_TIMER:
      return ("TIMER");
    case ESP_SLEEP_WAKEUP_TOUCHPAD:
      return ("TOUCHPAD");
    case ESP_SLEEP_WAKEUP_ULP:
      return ("ULP");
    case ESP_SLEEP_WAKEUP_GPIO:
      return ("GPIO");
    case ESP_SLEEP_WAKEUP_UART:
      return ("UART");
    default:
      return ("unknown");
  }
}

uint64_t start;

bool validTime(const tmElements_t &t) {
  if (t.Second >= 60) {
    return false;
  }
  if (t.Minute >= 60) {
    return false;
  }
  if (t.Hour >= 24) {
    return false;
  }
  if (t.Month > 12) {
    return false;
  }
  if (t.Day > 31) {
    return false;
  }
  if (t.Wday > 7) {
    return false;
  }
  return true;
}

bool fixTime(tmElements_t &t) {
  if (t.Second >= 60) {
    t.Second = 0;
  }
  if (t.Minute >= 60) {
    t.Minute = 0;
  }
  if (t.Hour >= 24) {
    t.Hour = 0;
  }
  if (t.Month > 12) {
    t.Month = 1;
  }
  if (t.Day > 31) {
    t.Day = 1;
  }
  if (t.Wday > 7) {
    t.Wday = 1;
  }
  return (RTC.write(t) == 0);
}

void initTime() {
  Wire.begin(SDA, SCL);  // init i2c

  // sync ESP32 clocks to RTC
  assert(RTC.read(currentTime) == 0);
  assert(validTime(currentTime) || fixTime(currentTime));
  setenv("TZ", Watchy_GetLocation::currentLocation.timezone, 1);
  tzset();
  time_t t = makeTime(currentTime);
  setTime(t);
  timeval tv = {t, 0};
  settimeofday(&tv, nullptr);
}

void init() {
  start = micros();
  Watchy_Event::BackgroundTask initTask("init", nullptr);
  initTask.add();
  esp_sleep_wakeup_cause_t wakeup_reason =
      esp_sleep_get_wakeup_cause();  // get wake up reason
  log_i("reason %s", wakeupReasonToString(wakeup_reason));
  initTime();

  for (auto &&owc : owcVec) {
    owc(wakeup_reason);
  }

  switch (wakeup_reason) {
    case ESP_SLEEP_WAKEUP_TIMER:  // ESP Internal RTC
#ifdef ESP_RTC
      tmElements_t currentTime;
      RTC.read(currentTime);
      currentTime.Minute++;
      tmElements_t tm;
      tm.Month = currentTime.Month;
      tm.Day = currentTime.Day;
      tm.Year = currentTime.Year;
      tm.Hour = currentTime.Hour;
      tm.Minute = currentTime.Minute;
      tm.Second = 0;
      time_t t = makeTime(tm);
      RTC.set(t);
#endif
      Watchy_Event::Event{
          .id = Watchy_Event::UPDATE_SCREEN,
          .micros = micros(),
      }.send();
      break;
    case ESP_SLEEP_WAKEUP_EXT0:  // RTC Alarm
      RTC.alarm(ALARM_1);        // resets the alarm flag in the RTC
      RTC.alarm(ALARM_2);        // resets the alarm flag in the RTC
      Watchy_Event::Event{
          .id = Watchy_Event::UPDATE_SCREEN,
          .micros = micros(),
      }.send();
      break;
    case ESP_SLEEP_WAKEUP_EXT1:  // button Press
      handleButtonPress();
      break;
    default:  // reset
      _rtcConfig();
      bmaConfig();
      showWatchFace(false);  // full update on reset
      break;
  }
  initTask.remove();
  do {
    Watchy_Event::handle();
  } while (Watchy_Event::BackgroundTask::running());
  Watchy::deepSleep();
}

void deepSleep() {
  uint64_t elapsed = micros() - start;
  display.hibernate();
  Watchy_Event::enableUpdateTimer();
  esp_sleep_enable_ext1_wakeup(
      BTN_PIN_MASK,
      ESP_EXT1_WAKEUP_ANY_HIGH);  // enable deep sleep wake on button press
  log_i("%6d *** sleeping after %llu.%03llums ***\n", millis(), elapsed / 1000,
        elapsed % 1000);
  esp_deep_sleep_start();
}

void _rtcConfig() {
#ifndef ESP_RTC
  // https://github.com/JChristensen/DS3232RTC
  // OE = Control
  // !EOSC | BBSQW | CONV | RS2 | RS1 | INTCN | A2IE | A1IE
  // !EOSC: Enable Oscillator
  // BBSQW: Battery-Backed Square-Wave Enable
  // CONV: Convert Temperature
  // RS2 | RS1: Rate Select in kHz (0 = 1, 1 = 1.024, 2 = 4.096, 3 = 8.192)
  // INTCN: Interrupt Control
  // A2IE: Alarm 2 Interrupt Enable
  // A1IE: Alarm 1 Interrupt Enable

  // 04 = Enable Oscillator | Enable Interrupts
  RTC.writeRTC(0x0E, 0X04);

  // OF = Control/Status
  // OSF | BB32kHz | CRATE1 | CRATE0 | EN32kHz | BSY | A2F | A1F
  // OSF: Oscillator Stop Flag
  // BB32kHz: Battery-Backed 32kHz Output
  // CRATE1 | CRATE0: Temperature Conversion Rate in Seconds (0=64, 1=128, 2=256, 3=512)
  // EN32kHz: Enable 32kHz Output
  // BSY: Busy
  // A2F: Alarm 2 Flag
  // A1F: Alarm 1 Flag

  // 00 = Clear OSF, disable 32kHz, Convert every 64 secs, clear alarms
  RTC.writeRTC(0x0F, 0X00);
#endif
}

void showWatchFace(bool partialRefresh, Screen *s) {
  display.init(
      0, false);  //_initial_refresh to false to prevent full update on init
  display.setFullWindow();
  display.setTextColor((s->bgColor == GxEPD_WHITE ? GxEPD_BLACK : GxEPD_WHITE));
  display.setCursor(0, 0);
  s->show();
  display.display(partialRefresh);  // partial refresh
}

const Screen *getScreen() { return screen; }

// setScreen is used to set a new screen on the display
void setScreen(Screen *s) {
  if (s == nullptr) {
    return;
  }
  screen = s;
  showWatchFace(true);
}

// This mutex protects the WiFi object, wifiConnectionCount
// and wifiReset
auto wifiMutex = xSemaphoreCreateMutex();
RTC_DATA_ATTR bool wifiReset = false;

bool connectWiFi() {
  // in theory this is re-entrant, but in practice if you call WiFi.begin()
  // while it's still trying to connect, it will return an error. Better
  // to serialize WiFi.begin()
  if (!wifiReset) {
    wifi_init_config_t wifi_config = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_config);
    wifiReset = true;
  }
#if !defined(WIFI_SSID) || !defined(WIFI_PASSWORD)
  if (WL_CONNECT_FAILED == WiFi.begin()) {
    // WiFi not setup, you can also use hard coded credentials with
    // WiFi.begin(SSID,PASS); by defining WIFI_SSID and WIFI_PASSWORD
#else
  if (WL_CONNECT_FAILED == WiFi.begin() &&
      WL_CONNECT_FAILED == WiFi.begin(WIFI_SSID, WIFI_PASSWORD)) {
    // WiFi not setup
#endif
    WIFI_CONFIGURED = false;
  } else {
    if (WL_CONNECTED ==
        WiFi.waitForConnectResult()) {  // attempt to connect for 10s
      WIFI_CONFIGURED = true;
    } else {  // connection failed, time out
      WIFI_CONFIGURED = false;
      // turn off radios
      WiFi.mode(WIFI_OFF);
      btStop();
    }
  }
  return WIFI_CONFIGURED;
}

unsigned int wifiConnectionCount = 0;

bool getWiFi() {
  xSemaphoreTake(wifiMutex, portMAX_DELAY);
  if (wifiConnectionCount == 0) {
    if (!connectWiFi()) {
      xSemaphoreGive(wifiMutex);
      return false;
    }
    log_d("wifi connected");
  }
  wifiConnectionCount++;
  xSemaphoreGive(wifiMutex);
  return true;
}

void releaseWiFi() {
  xSemaphoreTake(wifiMutex, portMAX_DELAY);
  wifiConnectionCount--;
  if (wifiConnectionCount == 0) {
    log_d("wifi disconnected");
    btStop();
    WiFi.mode(WIFI_OFF);
  }
  xSemaphoreGive(wifiMutex);
}
}  // namespace Watchy