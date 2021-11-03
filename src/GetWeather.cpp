#include "GetWeather.h"

#include <Arduino_JSON.h>

#include "GetLocation.h"
#include "Watchy.h"
#include "WatchyErrors.h"
#include "config.h"  // should be first

namespace Watchy_GetWeather {
RTC_DATA_ATTR weatherData currentWeather = {.temperature = 22,
                                            .weatherConditionCode = 800};
RTC_DATA_ATTR time_t lastGetWeatherTS = 0;

weatherData getWeather() {
  // only update if WEATHER_UPDATE_INTERVAL has elapsed i.e. 30 minutes
  if (lastGetWeatherTS &&
      (now() - lastGetWeatherTS < WEATHER_UPDATE_INTERVAL)) {
    // too soon to update, just re-use existing values. Not an error
    Watchy::err = Watchy::RATE_LIMITED;
    return currentWeather;
  }
  if (!Watchy::getWiFi()) {
    Watchy::err = Watchy::WIFI_FAILED;
    log_e("Wifi connect failed");
    // No WiFi, return RTC Temperature (this isn't actually useful...)
    uint8_t temperature = Watchy::RTC.temperature() / 4;  // celsius
    if (strcmp(TEMP_UNIT, "imperial") == 0) {
      temperature = temperature * 9. / 5. + 32.;  // fahrenheit
    }
    currentWeather.temperature = temperature;
    // we don't know, weatherConditionCode keep the last one
    return currentWeather;
  }

  // WiFi is connected Use Weather API for live data
  HTTPClient http;
  http.setConnectTimeout(10000);  // 10 second max timeout
  const unsigned int weatherQueryURLSize =
      strlen(OPENWEATHERMAP_URL) + strlen("?lat=") + 8 + strlen("&lon=") + 8 +
      strlen("&units=") + strlen(TEMP_UNIT) + strlen("&appid=") +
      strlen(OPENWEATHERMAP_APIKEY) + 1;
  char weatherQueryURL[weatherQueryURLSize];
  Watchy_GetLocation::getLocation(); // maybe update location?
  snprintf(weatherQueryURL, weatherQueryURLSize,
           "%s?lat=%.4f&lon=%.4f&units=%s&appid=%s", OPENWEATHERMAP_URL,
           Watchy_GetLocation::currentLocation.lat,
           Watchy_GetLocation::currentLocation.lon, TEMP_UNIT,
           OPENWEATHERMAP_APIKEY);
  if (!http.begin(weatherQueryURL)) {
    Watchy::err = Watchy::REQUEST_FAILED;
    log_e("http.begin failed");
  } else {
    int httpResponseCode = http.GET();
    if (httpResponseCode == 200) {
      String payload = http.getString();
      JSONVar responseObject = JSON.parse(payload);
      currentWeather.temperature = int(responseObject["main"]["temp"]);
      currentWeather.weatherConditionCode =
          int(responseObject["weather"][0]["id"]);
      strncpy(currentWeather.weatherCity, Watchy_GetLocation::currentLocation.city,
              sizeof(currentWeather.weatherCity));
      lastGetWeatherTS = now();
      Watchy::err = Watchy::OK;
    } else {
      Watchy::err = Watchy::REQUEST_FAILED;
      log_e("http response %d", httpResponseCode);
    }
    http.end();
  }
  Watchy::releaseWiFi();
  return currentWeather;
}

}  // namespace Watchy_GetWeather

/* weather response JSON prettified
{
    "coord": {
        "lon": 144.968,
        "lat": -37.8008
    },
    "weather": [
        {
            "id": 804,
            "main": "Clouds",
            "description": "overcast clouds",
            "icon": "04n"
        }
    ],
    "base": "stations",
    "main": {
        "temp": 11.76,
        "feels_like": 11.16,
        "temp_min": 9.78,
        "temp_max": 13.05,
        "pressure": 1026,
        "humidity": 83
    },
    "visibility": 10000,
    "wind": {
        "speed": 0.89,
        "deg": 228,
        "gust": 1.79
    },
    "clouds": {
        "all": 90
    },
    "dt": 1628326663,
    "sys": {
        "type": 2,
        "id": 2008797,
        "country": "AU",
        "sunrise": 1628284471,
        "sunset": 1628321843
    },
    "timezone": 36000,
    "id": 2171000,
    "name": "Collingwood",
    "cod": 200
}
*/
