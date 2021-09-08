#include "SyncTimeScreen.h"

#include "GetLocation.h"
#include "../Fonts/OptimaLTStd12pt7b.h"
#include "SyncTime.h"
#include "Watchy.h"
#include "WatchyErrors.h"

typedef enum { ready, waiting, success, fail, numStates } SyncStates;
RTC_DATA_ATTR SyncStates syncState = ready;

const char* syncStateMsgs[numStates] = {"\nready", "\nwaiting", "\nsuccess",
                                        "\fail"};

void printTime() {
  time_t tt = now();
  tm t;
  localtime_r(&tt, &t);
  Watchy::display.print(&t, "\nsuccess\n%I:%M %Z\n%A\n%b %e %Y");
}

void printSyncState() {
  if (syncState == success) {
    printTime();
    return;
  }
  Watchy::display.print(syncStateMsgs[syncState]);
  Watchy::display.print("\npress back to exit");
}

void SyncTimeScreen::show() {
  Watchy::display.fillScreen(bgColor);
  Watchy::display.setFont(OptimaLTStd12pt7b);
  if (syncState != ready) {
    printSyncState();
    return;
  }
  syncState = waiting;
  Watchy::display.print(syncStateMsgs[syncState]);
  Watchy::display.display(true);
  Watchy::display.fillScreen(bgColor);
  Watchy::display.setCursor(0, 0);

  Watchy_SyncTime::syncTime(Watchy_GetLocation::currentLocation.timezone);
  if (Watchy::err == Watchy::OK) {
    syncState = success;
  } else {
    syncState = fail;
  }
  printSyncState();
}

void SyncTimeScreen::back() {
  syncState = ready;
  Screen::back();
}
// RTC does not know about TZ
// so DST has to be in app code
// so RTC must store UTC
// RTC computes leap year so you want year to be correct
// according to how the RTC represents it