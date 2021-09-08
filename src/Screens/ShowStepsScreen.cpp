#include "ShowStepsScreen.h"

#include "../Fonts/OptimaLTStd22pt7b.h"
#include "Watchy.h"

void ShowStepsScreen::show() {
  Watchy::display.fillScreen(bgColor);
  Watchy::display.setFont(OptimaLTStd22pt7b);
  Watchy::display.printf("\n%d\nsteps", Watchy::sensor.getCounter());
}
