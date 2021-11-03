#pragma once

#include <time.h>
namespace Watchy_GetLocation {

typedef struct {
  float lat;
  float lon;
  const char *timezone; // POSIX tz spec https://developer.ibm.com/articles/au-aix-posix/
  char city[30]; // even if we get a longer name from the API, we'll truncate it to fit the sceen width
} location;

extern location currentLocation;
extern time_t lastGetLocationTS; // timestamp of last successful getLocation

// sends update event on success
void getLocation();
}  // namespace Watchy_GetLocation
