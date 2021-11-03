#pragma once

#include "Screen.h"

class SetupWifiScreen : public Screen {
 public:
  SetupWifiScreen(uint16_t bg = GxEPD_WHITE) : Screen(bg) {}
  void show() override;
};
