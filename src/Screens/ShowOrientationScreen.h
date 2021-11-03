#pragma once

#include "Screen.h"

 class ShowOrientationScreen : public Screen {
 private:
  bool showing;
  void showMe();

 public:
  ShowOrientationScreen(uint16_t bg = GxEPD_WHITE) : Screen(bg) {}
  void show() override;
};