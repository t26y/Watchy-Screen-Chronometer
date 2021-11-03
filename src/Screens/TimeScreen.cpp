#include "TimeScreen.h"

#include <time.h>

#include "../Fonts/northrup_serif8pt7b.h"
#include "../Images/WatchfaceBitmaps.h"

using namespace Watchy;

int fgColor;

// from https://github.com/spinalcode/Mickey
int getPixel(int16_t x, int16_t y, const uint8_t *bitmap) {
  int16_t imageWidth = pgm_read_byte(bitmap);
  int16_t byteWidth = (imageWidth + 7) / 8;
  return (pgm_read_byte(bitmap + 2 + y * byteWidth + x / 8) & (128 >> (x & 7)));
}

// from https://github.com/spinalcode/Mickey
// xx, yy: center of the image
// yOffset: vertical offset to the visual center of the image, measured from the
// bottom borderBitmap, borderColor: this bitmap will be drawn under the primary
// bitmap. It must have the same dimensions
void drawBitmapRotate(int xx, int yy, int yOffset, const uint8_t *bitmap,
                      const uint8_t *borderBitmap, unsigned int fAngle,
                      uint16_t color = GxEPD_BLACK,
                      uint16_t borderColor = GxEPD_WHITE) {
  int iWidth = pgm_read_byte(bitmap);
  int iHeight = pgm_read_byte(bitmap + 1);
  int hX = iWidth / 2;
  int hY = iHeight - yOffset;
  float angle = fAngle * PI / 180.0;

  int startX = -hX;
  int endX = startX + iWidth;
  int startY = -hY;
  int endY = startY + iHeight;

  assert(hX <= hY);
  int startAreaX = xx - hY;
  int startAreaY = yy - hY;
  int endAreaX = 200 - startAreaX;
  int endAreaY = 200 - startAreaY;

  for (int x = startAreaX; x < endAreaX; x++) {
    yield();
    for (int y = startAreaY; y < endAreaY; y++) {
      int ux = (x - xx) * cos(-angle) - (y - yy) * sin(-angle);
      int uy = (x - xx) * sin(-angle) + (y - yy) * cos(-angle);

      if (ux >= startX && ux < endX && uy >= startY && uy < endY) {
        if (!getPixel(ux + hX, uy + hY, bitmap)) {
          display.drawPixel(x, y, color);
        } else if (!getPixel(ux + hX, uy + hY, borderBitmap)) {
          display.drawPixel(x, y, borderColor);
        }
      }
    }
  }
}

void TimeScreen::drawDate(String dayOfWeek, String date, int hourAngle,
                          int minuteAngle) {
  // ensure date will not be blocked by hands
  int xAnchor, yAnchor;
  bool topShade = false;
  // angles at which the positions are obscured by the hands.
  // Determinted manually in Inksape.
  if ((hourAngle < 151 || hourAngle > 213) &&
      (minuteAngle < 146 || minuteAngle > 217)) {
        //center position
    xAnchor = 100;
    yAnchor = 140;
  } else if ((hourAngle < 52 || hourAngle > 144) &&
             (minuteAngle < 57 || minuteAngle > 134)) {
               //right position
    xAnchor = 147;
    yAnchor = 91;
    topShade = true;
  } else if ((hourAngle < 216 || hourAngle > 300) &&
             (minuteAngle < 224 || minuteAngle > 294)) {
               //left position
    xAnchor = 52;
    yAnchor = 91;
    topShade = true;
  }
// ignore warning about Anchors being uninitialized, above logic guarantees it
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"

  auto font = northrup_serif8pt7b;
  int16_t _x, _y;
  uint16_t w, h;
  display.setTextColor(fgColor);
  display.setTextSize(0);
  display.setFont(font);

  display.getTextBounds(dayOfWeek, 0, 0, &_x, &_y, &w, &h);
  int ys = yAnchor + h - 3;
  if (topShade) {
    display.setCursor(xAnchor - w / 2, ys + 1);
    display.setTextColor(bgColor);
    display.println(dayOfWeek);
    display.setTextColor(fgColor);
  }
  display.setCursor(xAnchor - w / 2, ys);
  display.println(dayOfWeek);

  display.getTextBounds(date, 0, 0, &_x, &_y, &w, &h);
  display.setCursor(xAnchor - w / 2, ys + font->yAdvance - 3);
  display.println(date);
#pragma GCC diagnostic pop
}

void TimeScreen::drawWatchface(tm t, float battery) {
  display.fillScreen(bgColor);
  drawRLEBitmap(display, 0, 0, &rle_background, fgColor);

  display.drawFastHLine(63, 54, (int)(battery * 74.0), fgColor);

  int hourAngle = ((t.tm_hour % 12) * 60 + t.tm_min) * 360 / 720;
  int minuteAngle = t.tm_min * 6;

  drawDate(dayStr(t.tm_wday + 1),
           String(t.tm_mday) + ". " + String(monthStr(t.tm_mon + 1)), hourAngle,
           minuteAngle);

  drawBitmapRotate(100, 100, 6, fat_bitmap_hour_hand,
                   fat_bitmap_hour_hand_border, hourAngle, fgColor, bgColor);
  drawBitmapRotate(100, 100, 8, fat_bitmap_minute_hand,
                   fat_bitmap_minute_hand_border, minuteAngle, fgColor,
                   bgColor);
}

void TimeScreen::show() {
  // show once every minute
  Watchy_Event::setUpdateInterval(SECS_PER_MIN*1000);

  fgColor = bgColor == GxEPD_WHITE ? GxEPD_BLACK : GxEPD_WHITE;

  tm t;
  time_t tt = now();
  localtime_r(&tt, &t);

  drawWatchface(t, (getBatteryVoltage() - 3.3) / 0.9);

  // demoMode();
}

// blocking demo mode that will run through all hand positions while decreasing
// the battery indicator and changing the date.
void TimeScreen::demoMode() {
  for (int h = 0; h < 12; h++) {
    for (int m = 0; m < 60; m++) {
      tm t = {.tm_sec = 0,
              .tm_min = m,
              .tm_hour = h,
              .tm_mday = m / 2,
              .tm_mon = h,
              .tm_year = 2021,
              .tm_wday = 3,
              .tm_yday = 129,
              .tm_isdst = 1};
      drawWatchface(t, (60.0 - (float)m) / 60.0);
      display.display(true);
    }
  }
}
