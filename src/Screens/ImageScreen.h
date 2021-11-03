#pragma once

#include "Adafruit_GFX.h"
#include "Screen.h"

class ImageScreen : public Screen {
 protected:
  const uint8_t *bitmap;
  const uint8_t width, height;
  const char *label;
  const GFXfont *font;

 public:
  ImageScreen(const uint8_t b[], const uint8_t w, const uint8_t h, const char *l,
              const GFXfont *f, uint16_t bg = GxEPD_WHITE)
      : Screen(bg), bitmap(b), width(w), height(h), label(l), font(f) {}
  void show() override {
    log_i("%s", label);
    Watchy_Event::setUpdateInterval(0);  // static screen
    const uint16_t fgColor =
        (bgColor == GxEPD_WHITE ? GxEPD_BLACK : GxEPD_WHITE);
    Watchy::display.setFont(font);
    int16_t x1, y1;
    uint16_t w, h;
    Watchy::display.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
    Watchy::display.fillScreen(bgColor);
    // even space between top<->icon<->label<->bottom
    int space = (DISPLAY_HEIGHT - (height + h)) / 3;
    Watchy::display.drawBitmap((DISPLAY_WIDTH - width) / 2, space, bitmap, width, height, fgColor);
    // want y1+h to be space above DISPLAY_HEIGHT
    Watchy::display.setCursor((DISPLAY_WIDTH - w) / 2, (DISPLAY_HEIGHT - space - (y1 + h)));
    Watchy::display.print(label);
  }
};
