#include "CarouselScreen.h"

#include "Watchy.h"

using namespace Watchy;

RTC_DATA_ATTR int8_t CarouselScreen::index;

CarouselScreen::CarouselScreen(CarouselItem *cis, const int8_t cs, uint16_t bg) : Screen(bg), items(cis), size(cs) {
  for (int i = 0; i < size; i++) {
    if (items[i].splash != nullptr) {
      items[i].splash->parent = this;
    }
    if (items[i].child != nullptr) {
      items[i].child->parent = this;
    }
  }
}

void CarouselScreen::show() {
  // Watchy::showWatchFace(true, items[index].splash);
  items[index].splash->show();
}

void CarouselScreen::menu() {
  if (items[index].child != nullptr) {
    Watchy::setScreen(items[index].child);
  }
}

void CarouselScreen::back() {
  index = 0;
  Watchy::showWatchFace(true);
}

void CarouselScreen::up() {
  index--;
  if (index < 0) {
    index = size - 1;
  }
  Watchy::showWatchFace(true);
}

void CarouselScreen::down() {
  index++;
  if (index >= size) {
    index = 0;
  }
  Watchy::showWatchFace(true);
}
