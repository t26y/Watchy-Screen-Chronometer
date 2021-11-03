#include "BuzzScreen.h"

#include "Watchy.h"

void BuzzScreen::show() {
  Watchy_Event::setUpdateInterval(0);
  pinMode(VIB_MOTOR_PIN, OUTPUT);
  // five cycles of 3 buzzes each
  for (int c = 0; c < 5; c++) {
    for (int b = 0; b < 3; b++) {
      digitalWrite(VIB_MOTOR_PIN, true);
      delay(100);
      digitalWrite(VIB_MOTOR_PIN, false);
      delay(100);
    }
    delay(600);
  }
  Screen::back(); // automatically return to parent
}