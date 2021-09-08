#include "ShowBatteryScreen.h"

#include <time.h>

#include "../Fonts/OptimaLTStd12pt7b.h"
#include "Watchy.h"

const int batteryReadingssz = 24 * 7;
RTC_DATA_ATTR float batteryReadings[batteryReadingssz];
RTC_DATA_ATTR int nextReading = 0;
RTC_DATA_ATTR time_t lastReadingTS = 0;
const time_t updateInterval = SECS_PER_HOUR;

void OnWake(esp_sleep_wakeup_cause_t wakeup_reason) {
  // setup
  if ((lastReadingTS != 0) && (now() < lastReadingTS + updateInterval)) {
    return;  // hasn't been an updateInterval yet
  }
  do {
    batteryReadings[nextReading] = Watchy::getBatteryVoltage();
    nextReading = (nextReading + 1) % batteryReadingssz;
    if (lastReadingTS == 0) {
      // first time
      lastReadingTS = now();
    } else {
      // take another reading one updateInterval after the last one
      lastReadingTS += updateInterval;
    }
    // catch up in case we slept for a very long time
  } while (now() >= lastReadingTS + updateInterval);
}

ShowBatteryScreen::ShowBatteryScreen(uint16_t bg) : Screen(bg) {
  Watchy::AddOnWakeCallback(&OnWake);
}

float mapRange(const float v, const float inMax, const float inMin,
               const float outMax, const float outMin) {
  return (min(inMax, max(inMin, v)) - inMin) / (inMax - inMin) *
             (outMax - outMin) +
         outMin;
}

void ShowBatteryScreen::show() {
  float fgColor = (bgColor == GxEPD_WHITE ? GxEPD_BLACK : GxEPD_WHITE);
  Watchy::display.fillScreen(bgColor);
  Watchy::display.setFont(OptimaLTStd12pt7b);
  Watchy::display.printf("\nbattery %5.3fv\n", Watchy::getBatteryVoltage());
  // draw the history graph
  const int16_t margin = 16;
  const int16_t height = 120;
  const int16_t width = batteryReadingssz;
  const int16_t x0 = margin + 1;  // left edge of graph
  const int16_t y0 = DISPLAY_HEIGHT - margin - height - 1;  // top of graph
  // draw a border around it
  Watchy::display.drawRect(x0 - 1, y0 - 1, width + 2, height + 2, fgColor);
  for (int i = 0; i < batteryReadingssz; i++) {
    const float v = batteryReadings[(nextReading + i) % batteryReadingssz];
    if (v > 0.0) {
      // map [4200 .. 3200) onto [100..0)
      const int h = int(mapRange(v, 4.200, 3.200, height, 0.0) + 0.5);
      Watchy::display.drawLine(x0 + i, y0 + height - h, x0 + i, y0 + height,
                               fgColor);
    }
  }
}
