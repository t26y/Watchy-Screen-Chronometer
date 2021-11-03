#include "MenuScreen.h"

#include "Watchy.h"
#include "Events.h"

using namespace Watchy;

RTC_DATA_ATTR int8_t MenuScreen::first;
RTC_DATA_ATTR int8_t MenuScreen::index;

MenuScreen::MenuScreen(MenuItem *mis, const int8_t ms, const GFXfont *f,
                       uint16_t bg)
    : Screen(bg), items(mis), size(ms), font(f) {
  for (int i = 0; i < size; i++) {
    if (items[i].screen != nullptr) {
      items[i].screen->parent = this;
    }
  }
}

void MenuScreen::show() {
  log_i("%s", items[index].name);
  Watchy_Event::setUpdateInterval(0);  // static screen
  const uint16_t fgColor =
      (screen->bgColor == GxEPD_WHITE ? GxEPD_BLACK : GxEPD_WHITE);
  Watchy::display.fillScreen(bgColor);
  display.setFont(font);
  const uint16_t menuLineHeight = font->yAdvance * 15 / 10; // 50% padding
  int16_t yPos = 0;
  for (uint8_t i = 0; i < min(size, maxItemsOnScreen()); i++) {
    yPos += menuLineHeight;
    if (first + i == index) {
      int16_t x1, y1;
      uint16_t w, h;
      Watchy::display.getTextBounds(items[first + i].name, 0, yPos, &x1, &y1,
                                    &w, &h);
      display.fillRect(0, y1 - (menuLineHeight - h) / 2, DISPLAY_WIDTH,
                       menuLineHeight, fgColor);
      display.setTextColor(bgColor);
    } else {
      display.setTextColor(fgColor);
    }
    display.setCursor(0, yPos);
    display.println(items[first + i].name);
    const uint8_t triangleWidth = 19;
    const uint8_t trianglePad = 5;
    const uint8_t triangleHeight = (triangleWidth*866+500)/1000; // sqrt(triangleWidth^2-(triangleWidth/2)^2)
    if (first > 0) {
      // draw scroll up arrow
      const uint16_t color = (first == index ? bgColor : fgColor);
      Watchy::display.fillTriangle(
          DISPLAY_WIDTH - triangleWidth - trianglePad, triangleHeight + trianglePad, 
          DISPLAY_WIDTH - trianglePad, triangleHeight + trianglePad,
          DISPLAY_WIDTH - triangleWidth / 2 - trianglePad, trianglePad,
          color);
    }
    if (first < size - maxItemsOnScreen()) {
      // draw scroll down arrow
      const uint16_t color =
          (first + maxItemsOnScreen() - 1 == index ? bgColor : fgColor);
      Watchy::display.fillTriangle(
          DISPLAY_WIDTH - triangleWidth - trianglePad, DISPLAY_HEIGHT - triangleHeight - trianglePad, 
          DISPLAY_WIDTH - trianglePad, DISPLAY_HEIGHT - triangleHeight - trianglePad,
          DISPLAY_WIDTH - triangleWidth / 2 - trianglePad, DISPLAY_HEIGHT - trianglePad,
          color);
    }
  }
}

void MenuScreen::menu() { Watchy::setScreen(items[index].screen); }

void MenuScreen::back() { setScreen(parent); }

void MenuScreen::up() {
  index--;
  if (index < 0) {
    index = 0;
  }
  if (index < first) {
    first = index;
  }
  showWatchFace(true);
}

void MenuScreen::down() {
  index++;
  if (index >= size) {
    index = size - 1;
  }
  if (index >= first + maxItemsOnScreen()) {
    first = index - maxItemsOnScreen() + 1;
  }
  showWatchFace(true);
}

uint8_t MenuScreen::maxItemsOnScreen() const {
  const uint16_t menuLineHeight = font->yAdvance * 15 / 10;
  return DISPLAY_HEIGHT / menuLineHeight;
}
