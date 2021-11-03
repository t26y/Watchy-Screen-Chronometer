#include "SetupWifiScreen.h"

#include "../Fonts/../Fonts/OptimaLTStd12pt7b.h"
#include "Screen.h"
#include "Watchy.h"

const int WIFI_AP_TIMEOUT = 60;

void _configModeCallback(WiFiManager *myWiFiManager) {
  Watchy::display.println("Connect to");
  Watchy::display.print("SSID: ");
  Watchy::display.println(WIFI_AP_SSID);
  Watchy::display.print("IP: ");
  Watchy::display.println(WiFi.softAPIP());
  Watchy::display.display(true);
}

void SetupWifiScreen::show() {
  Watchy_Event::setUpdateInterval(0);
  Watchy::display.fillScreen(bgColor);
  Watchy::display.setFont(OptimaLTStd12pt7b);
  Watchy::display.setCursor(0, 30);

  WiFiManager wifiManager;
  wifiManager.resetSettings();
  wifiManager.setTimeout(WIFI_AP_TIMEOUT);
  wifiManager.setAPCallback(_configModeCallback);
  Watchy::WIFI_CONFIGURED = wifiManager.autoConnect(WIFI_AP_SSID);

  Watchy::display.fillScreen(bgColor);
  Watchy::display.setFont(OptimaLTStd12pt7b);
  Watchy::display.setCursor(0, 30);

  if (Watchy::WIFI_CONFIGURED) {
    Watchy::display.println("Success!");
    Watchy::display.println("Connected to");
    Watchy::display.println(WiFi.SSID());
    Watchy::display.println("press back");
    Watchy::display.println("to return to menu");
  } else {
    Watchy::display.println("Wifi setup");
    Watchy::display.println("failed");
    Watchy::display.println("Connection");
    Watchy::display.println("timed out!");
    Watchy::display.println("press back");
    Watchy::display.println("to return to menu");
  }
  btStop();
  WiFi.mode(WIFI_OFF);
}