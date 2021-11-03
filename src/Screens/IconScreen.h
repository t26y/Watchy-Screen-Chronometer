#pragma once

#include "Adafruit_GFX.h"
#include "Screen.h"
#include "rle.h"

class IconScreen : public Screen {
 protected:
  const rle *rleImage;
  const char *label;
  const GFXfont *font;

 public:
  IconScreen(const rle *r, const char *l, const GFXfont *f, uint16_t bg = GxEPD_WHITE) : Screen(bg), rleImage(r), label(l), font(f) {}
  void show() override {
    log_i("%s", label);
    Watchy_Event::setUpdateInterval(0); // static screen
    const uint16_t fgColor =
        (bgColor == GxEPD_WHITE ? GxEPD_BLACK : GxEPD_WHITE);
    Watchy::display.setFont(font);
    int16_t x1, y1;
    uint16_t w, h;
    Watchy::display.getTextBounds(label, 0, 0, &x1, &y1, &w, &h);
    Watchy::display.fillScreen(bgColor);
    // even space between top<->icon<->label<->bottom
    int space = (DISPLAY_HEIGHT - (rleImage->h + h)) / 3;
    drawRLEBitmap(Watchy::display, (DISPLAY_WIDTH - rleImage->w) / 2, space, rleImage,
                  fgColor);
    // want y1+h to be space above DISPLAY_HEIGHT
    Watchy::display.setCursor((DISPLAY_WIDTH - w) / 2, (DISPLAY_HEIGHT - space - (y1 + h)));
    Watchy::display.print(label);
  }
};
