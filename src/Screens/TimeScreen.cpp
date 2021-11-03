#include "TimeScreen.h"

#include <stdlib.h>
#include <time.h>

#include "OptimaLTStd22pt7b.h"
#include "OptimaLTStd7pt7b.h"
#include "OptimaLTStd_Black32pt7b.h"
#include "GetLocation.h"

using namespace Watchy;

// inspired by http://rosettacode.org/wiki/Number_names#C.2B.2B
const char *smallNumbers[] = {"",        "one",       "two",      "three",
                              "four",    "five",      "six",      "seven",
                              "eight",   "nine",      "ten",      "eleven",
                              "twelve",  "thirteen",  "fourteen", "fifteen",
                              "sixteen", "seventeen", "eighteen", "nineteen"};
const char *decades[] = {"oh", nullptr, "twenty", "thirty", "forty", "fifty"};

void rightJustify(const char *txt, uint16_t &yPos) {
  int16_t x1, y1;
  uint16_t w, h;
  const uint8_t PADDING = 0; // how much padding to leave around text
  display.getTextBounds(txt, 0, 0, &x1, &y1, &w, &h);
  // right justify with padding
  display.setCursor(200-x1-w-PADDING, yPos);
  display.print(txt);  
}

void TimeScreen::show() {
  Watchy_Event::setUpdateInterval(SECS_PER_MIN*1000);
  tm t;
  time_t tt = now();
  localtime_r(&tt, &t);

  Watchy::display.fillScreen(bgColor);

  // hours
  const GFXfont * font = OptimaLTStd_Black32pt7b;
  display.setFont(font);
  uint16_t yPos = font->yAdvance; // assume cursor(0,0)
  rightJustify(smallNumbers[(t.tm_hour + 11) % 12 + 1], yPos);

  // minutes
  font = OptimaLTStd22pt7b;
  display.setFont(font);
  yPos += font->yAdvance;
  const char *txt;
  assert(t.tm_min >= 0 && t.tm_min < 60);
  if (t.tm_min == 0) {
    // 0: exactly on the hour
    if (t.tm_hour == 0) {
      txt = "midnight";
    } else if (t.tm_hour == 12) {
      txt = "noon";
    } else {
      txt = "o'clock";
    }
  } else if (10 <= t.tm_min && t.tm_min < 20) {
    // 10-19
    txt = smallNumbers[t.tm_min];
  } else if (t.tm_min <= 59) {
    // 1-9, 20-59
    rightJustify(decades[t.tm_min / 10], yPos);
    yPos += font->yAdvance;
    txt = smallNumbers[t.tm_min % 10];
  }
  // ignore warning about txt not initialized, assert guarantees it will be
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
  rightJustify(txt, yPos);
  #pragma GCC diagnostic pop

  // date
  display.setCursor(0, 195);
  display.setFont(OptimaLTStd7pt7b);
  display.print(&t, "%a, %B %d %Y %Z");
  loop(); // TESTING
}