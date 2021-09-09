#pragma once

#include "Screen.h"

class TimeScreen : public Screen {
 public:
  TimeScreen(uint16_t bg = GxEPD_WHITE) : Screen(bg) {}
  void show() override;
  void drawWatchface(tm t, float battery);
  void demoMode();
  void drawDate(String dayOfWeek, String date, int hourAngle, int minuteAngle);
};