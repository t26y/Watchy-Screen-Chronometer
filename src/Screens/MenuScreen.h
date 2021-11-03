#pragma once

#include "Adafruit_GFX.h"
#include "../Fonts/../Fonts/OptimaLTStd12pt7b.h"
#include "Screen.h"

typedef struct {
  const char *name;
  Screen *screen;
} MenuItem;

class MenuScreen : public Screen {
 private:
  const MenuItem *items;  // array of MenuItems
  const uint8_t size;     // number of MenuItems
  static int8_t first;    // first MenuItem being displayed
  static int8_t index;    // currently highlighted MenuItem
  const GFXfont *font;

 public:
  MenuScreen(MenuItem *mis, const int8_t ms,
             const GFXfont *font = OptimaLTStd12pt7b,
             uint16_t bg = GxEPD_WHITE);
  void show() override;
  void menu() override;
  void back() override;
  void up() override;
  void down() override;
  uint8_t maxItemsOnScreen() const;
};