#include "SetupWifiScreen.h"

#include "../Fonts/OptimaLTStd12pt7b.h"
#include "Screen.h"
#include "Watchy.h"

using namespace Watchy;

const int WIFI_AP_TIMEOUT = 60;

RTC_DATA_ATTR enum WifiSetupState { start, success, fail } wifiSetupState;

void _configModeCallback(WiFiManager *myWiFiManager) {
  display.println("Connect to");
  display.print("SSID: ");
  display.println(WIFI_AP_SSID);
  display.print("IP: ");
  display.println(WiFi.softAPIP());
  display.display(true);
  display.hibernate();
}

void SetupWifiScreen::show() {
  // this is a little ugly the show() method blocks without
  // showing anything in autoConnect, and lets _configModeCallback
  // set the display
  Watchy::display.fillScreen(bgColor);
  display.setFont(OptimaLTStd12pt7b);
  display.setCursor(0, 30);

  switch (wifiSetupState) {
    case success:
      display.println("Wifi setup");
      display.println("succeeded");
      if (WIFI_CONFIGURED) {
        display.println("Connected to");
        display.println(WiFi.SSID());
      }
      display.println("press back");
      display.println("to return to menu");
      break;

    case fail:
      display.println("Wifi setup");
      display.println("failed");
      display.println("Connection");
      display.println("timed out!");
      display.println("press back");
      display.println("to return to menu");
      break;

    default:
      WiFiManager wifiManager;
      wifiManager.resetSettings();
      wifiManager.setTimeout(WIFI_AP_TIMEOUT);
      wifiManager.setAPCallback(_configModeCallback);

      // now we just wait for <timeout> to see if we connect?
      if (!wifiManager.autoConnect(WIFI_AP_SSID)) {
        WIFI_CONFIGURED = false;
        wifiSetupState = fail;
      } else {
        WIFI_CONFIGURED = true;
        wifiSetupState = success;
      }
      // turn off radios
      WIFI_CONFIGURED = false;
      WiFi.mode(WIFI_OFF);
      btStop();
      break;
  }

  display.display(true);
  display.hibernate();
}

void SetupWifiScreen::back() {
  wifiSetupState = start;
  Screen::back();
}
