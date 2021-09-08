#include "SetTimeScreen.h"

#include "../Fonts/OptimaLTStd_Black32pt7b.h"
#include "../Fonts/OptimaLTStd12pt7b.h"
#include "Watchy.h"

using namespace Watchy;

const int SET_HOUR = 0;
const int SET_MINUTE = 1;
const int SET_YEAR = 2;
const int SET_MONTH = 3;
const int SET_DAY = 4;

bool blink;
bool revert;
bool commit;

RTC_DATA_ATTR uint8_t SetTimeScreen::setIndex;

typedef struct {
  int16_t SetTimeScreen::*val;
  int16_t min;
  int16_t max;
} Field;

void decrMin(int16_t &val, int16_t &min, int16_t &max) {
  if (val-- <= min) {
    val = max;
  }
}

void incrMax(int16_t &val, int16_t &min, int16_t &max) {
  if (val++ >= max) {
    val = min;
  }
}

Field fields[] = {{&SetTimeScreen::hour,   0,  23},
                  {&SetTimeScreen::minute, 0,  59},
                  {&SetTimeScreen::year, 120, 199},
                  {&SetTimeScreen::month,  1,  12},
                  {&SetTimeScreen::day,    1,  31}};
const uint8_t numFields = sizeof(fields) / sizeof(fields[0]);

void SetTimeScreen::show() {
  struct tm tm;
  time_t t = now();
  localtime_r(&t, &tm);

  minute = tm.tm_min;
  hour = tm.tm_hour;
  day = tm.tm_mday;
  month = tm.tm_mon;
  year = tm.tm_year;

  blink = 0;
  revert = false;
  commit = false;

  pinMode(DOWN_BTN_PIN, INPUT);
  pinMode(UP_BTN_PIN, INPUT);
  pinMode(MENU_BTN_PIN, INPUT);
  pinMode(BACK_BTN_PIN, INPUT);

  while (!revert && !commit) {
    while (!pollButtonsAndDispatch()) {
      blink = 1 - blink;
      const uint16_t fgColor =
          (screen->bgColor == GxEPD_WHITE ? GxEPD_BLACK : GxEPD_WHITE);

      display.fillScreen(screen->bgColor);
      display.setFont(OptimaLTStd_Black32pt7b);
      display.setTextColor(fgColor);

      display.setCursor(25, 80);
      if (setIndex == SET_HOUR) {  // blink hour digits
        display.setTextColor(blink ? fgColor : screen->bgColor);
      }
      display.printf("%2d", hour);

      display.setTextColor(fgColor);
      display.print(":");

      if (setIndex == SET_MINUTE) {  // blink minute digits
        display.setTextColor(blink ? fgColor : screen->bgColor);
      }
      display.printf("%02d", minute);

      display.setTextColor(fgColor);

      display.setFont(OptimaLTStd12pt7b);
      display.setCursor(50, 150);
      if (setIndex == SET_YEAR) {  // blink minute digits
        display.setTextColor(blink ? fgColor : screen->bgColor);
      }
      display.print(year + 1900);

      display.setTextColor(fgColor);
      display.print("/");

      if (setIndex == SET_MONTH) {  // blink minute digits
        display.setTextColor(blink ? fgColor : screen->bgColor);
      }
      display.printf("%02d", month);

      display.setTextColor(fgColor);
      display.print("/");

      if (setIndex == SET_DAY) {  // blink minute digits
        display.setTextColor(blink ? fgColor : screen->bgColor);
      }
      display.printf("%02d", day);
      display.display(true);  // partial refresh
    }
    yield();
  }

  if (commit) {
    // fudge factor to allow for upload time, etc. (seconds, YMMV)
    const time_t FUDGE(10);
    t = mktime(&tm) + FUDGE;
    setTime(t);
    RTC.set(t);
    timeval tv = {t, 0};
    settimeofday(&tv, nullptr);
  }
  setIndex = 0;  // make sure we start at the beginning if we come back

  Watchy::setScreen(parent);
}

void SetTimeScreen::up() {
  decrMin(this->*(fields[setIndex].val), fields[setIndex].min,
          fields[setIndex].max);
}

void SetTimeScreen::down() {
  incrMax(this->*(fields[setIndex].val), fields[setIndex].min,
          fields[setIndex].max);
}

void SetTimeScreen::back() {
  if (setIndex == 0) {
    revert = true;
  } else {
    setIndex--;
  }
}

void SetTimeScreen::menu() {
  if (setIndex == numFields - 1) {
    commit = true;
  } else {
    setIndex++;
  }
}