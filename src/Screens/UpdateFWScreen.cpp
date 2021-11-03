#include "UpdateFWScreen.h"

#include <freertos/task.h>

#include "OptimaLTStd12pt7b.h"
#include "Screen.h"
#include "Watchy.h"

RTC_DATA_ATTR int currentStatus; // ugly, should be a member

class : public Screen {
 private:
  // this local class variable only works because we
  // never deep sleep while doing BLE OTA
  BLE BT;

 public:
  void show() {
    Watchy_Event::setUpdateInterval(0);
    BT.begin("Watchy BLE OTA");
    TickType_t ticks = xTaskGetTickCount();
    for (;;) {
      int status = BT.getStatus();
      if (currentStatus != status || currentStatus == 1) {
        currentStatus = status;
        Watchy::display.setTextColor(
            (bgColor == GxEPD_WHITE ? GxEPD_BLACK : GxEPD_WHITE));
        Watchy::display.setCursor(0, 0);
        Watchy::display.fillScreen(bgColor);
        Watchy::display.setFont(OptimaLTStd12pt7b);
        switch (BT.getStatus()) {
          case 0:
            Watchy::display.println("\nBLE Connected!");
            Watchy::display.println();
            Watchy::display.println("Waiting for");
            Watchy::display.println("upload...");
            break;
          case 1:
            Watchy::display.println("\nDownloading");
            Watchy::display.println("firmware:");
            Watchy::display.println();
            Watchy::display.printf("%d bytes", BT.getBytesReceived());
            break;
          case 2:
            Watchy::display.println("\nDownload");
            Watchy::display.println("completed!");
            Watchy::display.println();
            Watchy::display.println("Press menu to");
            Watchy::display.println("reboot");
            return;
          case 4:
            Watchy::display.println("\nBLE Disconnected!");
            Watchy::display.println();
            Watchy::display.println("Press back to");
            Watchy::display.println("exit");
            return;
          default:
            Watchy::display.println("\nBluetooth Started");
            Watchy::display.println();
            Watchy::display.println("Watchy BLE OTA");
            Watchy::display.println();
            Watchy::display.println("Waiting for");
            Watchy::display.println("connection...");
            log_i("waiting for BT");
            break;
        }
        Watchy::display.display(true);
      }
      vTaskDelayUntil(&ticks, pdMS_TO_TICKS(1000));
    }
  }

  void menu() override {
    if (currentStatus == 2) {
      esp_restart();  // does not return
    }
  }

  void back() override {
    // turn off radios
    WiFi.mode(WIFI_OFF);
    btStop();
    Watchy::setScreen(parent);
  }
} updateFWBeginScreen;

void UpdateFWScreen::show() {
  Watchy_Event::setUpdateInterval(0);
  Watchy::display.fillScreen(bgColor);
  Watchy::display.setFont(OptimaLTStd12pt7b);
  Watchy::display.println("\nVisit");
  Watchy::display.println("watchy.sqfmi.com");
  Watchy::display.println("in a Bluetooth");
  Watchy::display.println("enabled browser");
  Watchy::display.println("On USB power");
  Watchy::display.println("Then press menu");
}

void UpdateFWScreen::menu() { Watchy::setScreen(&updateFWBeginScreen); }