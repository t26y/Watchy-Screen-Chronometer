#include "GetWeatherScreen.h"

#include <Arduino_JSON.h>

#include "GetWeather.h"
#include "../Fonts/OptimaLTStd12pt7b.h"
#include "Watchy.h"

RTC_DATA_ATTR enum GetWeatherState {
  ready,
  waiting,
  done
} getWeatherState = ready;

void printWeather(Watchy_GetWeather::weatherData wd) {
  Watchy::display.printf("\nsuccess\nTemp %d\nWeather %d", wd.temperature,
                         wd.weatherConditionCode);
}

void GetWeatherScreen::show() {
  Watchy_Event::setUpdateInterval(0);
  Watchy::display.fillScreen(bgColor);
  Watchy::display.setFont(OptimaLTStd12pt7b);
  switch (getWeatherState) {
    case ready: {
      getWeatherState = waiting;
      Watchy::display.print("\nwaiting");
      Watchy::display.display(true);
      Watchy::display.fillScreen(bgColor);
      Watchy::display.setCursor(0, 0);
      auto weather = Watchy_GetWeather::getWeather();
      getWeatherState = done;
      printWeather(weather);
      break;
    }
    case waiting:
      Watchy::display.print("\nwaiting");
      break;
    case done:
      printWeather(Watchy_GetWeather::getWeather());
      break;
    default:
      return;
  }
  Watchy::display.print("\npress back to exit");
}

void GetWeatherScreen::back() {
  getWeatherState = ready;
  Screen::back();
}
