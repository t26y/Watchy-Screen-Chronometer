#include "ShowWifiScreen.h"

#include "../Fonts/../Fonts/OptimaLTStd12pt7b.h"
#include "Watchy.h"

void ShowWifiScreen::show() {
  Watchy_Event::setUpdateInterval(0);
  bool connected = false;
  Watchy::display.fillScreen(bgColor);
  Watchy::display.setFont(OptimaLTStd12pt7b);
  auto status = WiFi.status();
  if (status == WL_NO_SHIELD) { // haven't yet connected this wake cycle
    Watchy::display.print("\nconnecting...");
    Watchy::display.display(true);
    Watchy::display.fillScreen(bgColor);
    Watchy::display.setCursor(0, 0);

    // can take up to 10 seconds to time out
    // ignore the result, we'll pick it up below
    connected = Watchy::getWiFi();
    status = WiFi.status();
  }

  Watchy::display.print("\nStatus ");
  switch (status) {
    case WL_NO_SHIELD:
      Watchy::display.print("no shield");
      return;
    case WL_IDLE_STATUS:
      Watchy::display.print("idle");
      return;
    case WL_NO_SSID_AVAIL:
      Watchy::display.print("no ssid");
      return;
    case WL_SCAN_COMPLETED:
      Watchy::display.print("scan complete");
      return;
    case WL_CONNECTED:
      Watchy::display.print("connected");
      break;
    case WL_CONNECT_FAILED:
      Watchy::display.print("connect failed");
      return;
    case WL_CONNECTION_LOST:
      Watchy::display.print("connection lost");
      return;
    case WL_DISCONNECTED:
      Watchy::display.print("disconnected");
      return;
    default:
      Watchy::display.print("unknown");
      return;
  }

  uint8_t primaryChan;
  wifi_second_chan_t secondChan;
  esp_wifi_get_channel(&primaryChan, &secondChan);

  Watchy::display.printf("\nChannel: %d", primaryChan);

  wifi_config_t conf;
  esp_wifi_get_config(WIFI_IF_STA, &conf);

  const char* ssid = reinterpret_cast<const char*>(conf.sta.ssid);
  Watchy::display.printf("\nSSID: %.*s", strnlen(ssid, sizeof(conf.sta.ssid)),
                         ssid);
  Watchy::display.print("\nPassword ");
  if (conf.sta.password[0] != 0) {
    Watchy::display.print("ok");
  } else {
    Watchy::display.print("not set");
  }
  Watchy::display.printf("\nBSSID set: %d", conf.sta.bssid_set);

  if (connected) {
    Watchy::releaseWiFi();
  }
}