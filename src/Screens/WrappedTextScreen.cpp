#include "WrappedTextScreen.h"

#include "GFXTextUtil.h"
#include "../Fonts/OptimaLTStd7pt7b.h"
#include "Watchy.h"

void WrappedTextScreen::show() {
  const int16_t margin = 20;
  const uint16_t fgColor = (bgColor == GxEPD_WHITE ? GxEPD_BLACK : GxEPD_WHITE);
  Watchy::display.fillScreen(bgColor);
  Watchy::display.drawRect(margin - 2, margin - 2,
                           DISPLAY_WIDTH - 2 * margin + 2,
                           DISPLAY_HEIGHT - 2 * margin + 2, fgColor);
  drawWordWrappedText(Watchy::display, margin, margin,
                      DISPLAY_WIDTH - 2 * margin, DISPLAY_HEIGHT - 2 * margin,
                      text, OptimaLTStd7pt7b);
}