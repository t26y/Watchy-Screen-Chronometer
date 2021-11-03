#include "ShowStepsScreen.h"

#include "../Fonts/OptimaLTStd22pt7b.h"
#include "Watchy.h"

void ShowStepsScreen::show() {
  Watchy_Event::setUpdateInterval(SECS_PER_MIN*5*1000);
  Watchy::display.fillScreen(bgColor);
  Watchy::display.setFont(OptimaLTStd22pt7b);
  Watchy::display.printf("\n%d\nsteps", Watchy::sensor.getCounter());
}
