#pragma once

#include "Screen.h"

class OTAScreen : public Screen {
 public:
  OTAScreen(uint16_t bg = GxEPD_WHITE) : Screen(bg) {}
  void show() override;
};