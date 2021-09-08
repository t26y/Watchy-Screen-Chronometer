#include "UpdateFWScreen.h"

#include "../Fonts/OptimaLTStd12pt7b.h"
#include "Screen.h"
#include "Watchy.h"

using namespace Watchy;

class : public Screen {
  void show() {
    Watchy::display.fillScreen(bgColor);
    display.setFont(OptimaLTStd12pt7b);
    display.println("\nBLE Connected!");
    display.println();
    display.println("Waiting for");
    display.println("upload...");
  }
} updateFWConnectedScreen;

class : public Screen {
  void show() {
    Watchy::display.fillScreen(bgColor);
    display.setFont(OptimaLTStd12pt7b);
    display.println("\nDownload");
    display.println("completed!");
    display.println();
    display.println("Rebooting...");
  }
} updateFWDownloadCompleteScreen;

class : public Screen {
  void show() {
    Watchy::display.fillScreen(bgColor);
    display.setFont(OptimaLTStd12pt7b);
    display.println("\nBLE Disconnected!");
    display.println();
    display.println("exiting...");
  }
} updateFWDisconnectedScreen;

void btPoll(Screen *s) {
  BLE BT;

  // this local class variable only works because we
  // never deep sleep while doing BLE OTA
  class updateFWDownloadingScreen : public Screen {
   private:
    BLE &BT;

   public:
    updateFWDownloadingScreen(BLE &bt, uint16_t bg = GxEPD_WHITE) : Screen(bg), BT(bt) {}
    void show() {
      Watchy::display.fillScreen(bgColor);
      display.setFont(OptimaLTStd12pt7b);
      display.println("\nDownloading");
      display.println("firmware:");
      display.println();
      display.printf("%d bytes", BT.howManyBytes());
    }
  } updateFWDownloadingScreen(BT);

  BT.begin("Watchy BLE OTA");

  int prevStatus = -1;
  int currentStatus;

  // only exits on download complete, or disconnect
  while (1) {
    currentStatus = BT.updateStatus();
    if (prevStatus != currentStatus) {
      switch (currentStatus) {
        case 0:
          setScreen(&updateFWConnectedScreen);
          break;
        case 1:
          setScreen(&updateFWDownloadingScreen);
          break;
        case 2:
          setScreen(&updateFWDownloadCompleteScreen);
          delay(2000);
          esp_restart();  // does not return
        case 4:
          setScreen(&updateFWDisconnectedScreen);
          delay(1000);

          // turn off radios
          WiFi.mode(WIFI_OFF);
          btStop();
          setScreen(s->parent);
          return;
        default:
          break;
      }
      prevStatus = currentStatus;
      delay(100);  // feed the watchdog
    } else if (prevStatus == 1) {
      showWatchFace(true);
      delay(1000); // update progress every 1 sec
    }
    yield();
  }
}

class : public Screen {
  void show() {
    Watchy::display.fillScreen(bgColor);
    display.setFont(OptimaLTStd12pt7b);
    display.println("\nBluetooth Started");
    display.println();
    display.println("Watchy BLE OTA");
    display.println();
    display.println("Waiting for");
    display.println("connection...");

    btPoll(this); // smells like a private method
  }
} updateFWBeginScreen;

void UpdateFWScreen::show() {
  Watchy::display.fillScreen(bgColor);
  display.setFont(OptimaLTStd12pt7b);
  display.println("\nPlease Visit");
  display.println("watchy.sqfmi.com");
  display.println("with a Bluetooth");
  display.println("enabled device");
  display.println();
  display.println("Press menu button");
  display.println("again when ready");
  display.println();
  display.println("Keep USB powered");

  BLE BT;
}

void UpdateFWScreen::menu() { setScreen(&updateFWBeginScreen); }