# Release Notes

* Asynchronous events and background tasks
* Interrupts and Events for button presses
* Variable screen refresh rates
* Change how RTC "on wake" alarms are handled
* OTA uploads
* Refactor BLE classes for better encapsulation
* Refactor sensor into its own files

## Breaking changes

* Must call `Watchy_Event::start()` in `setup()` before calling `Watchy::init()`
* Must call `Watchy_Event::setUpdateInterval(msecs)` in `Screen::show()`

## Asynchronous events

This is the biggest change. Watchy-Screen is now (mostly) event driven. That means timers and button presses happen asynchronously and are reported via an event queue. This also means that things like NTP updates, location updates, and weather updates can happen as background tasks. This change should be invisible if you don't want to take advantage of it.

## Background tasks

With asynchronous events come background tasks. You can now create a background task object, which is just a void function with a name. You can start them and kill them, but normally they run to completion and return. They run as ESP32 tasks. Because I've found that the ESP32 code and especially the Arduino code (I'm looking at you Wire) isn't reliably reentrant I've pinned the task to the "app" core, and I recommend both not using the I2C bus in your background tasks, and communicating with the main task by sending Events.

As an example, I've made main spawn background tasks to fetch the time and location the first time it starts after rebooting. Note that you now must call `Watchy_Event::start()` in your `setup()` function. I may try to get rid of that requirement in the future.

## Button interrupts

Buttons are now handled entirely using interrupts, so no more polling for button presses! Button debouncing happens automatically in the button interrupt routine. Because the event loop is single threaded you cannot currently get a button press while your `show()` method is running. `show()` methods that perform long running operations should either use existing asynchronous mechanisms (like `HTTPClient` or `configTzTime`) or a background task. See `GetLocation` and `SyncTime` for examples.

## Variable screen refresh rates

Screens can (must) say how often they want to be refreshed. In their `show()` methods by calling `Watchy_Event::setUpdateInterval(msecs)` 0 is a legal value meaning "static display, never refresh." To get the existing behaviour put `Watchy_Event::setUpdateInterval(SECS_PER_MIN*1000)` as the first line of your `Screen::show()` method. Note that screen updates take a minimum of about 300ms.

## Change how RTC "on wake" alarms are handled

In order to support variable screen refresh rates, RTC alarms have been changed to trigger at the *end* of the current "updateInterval". So if your update interval was 600000 ms (every 10 minutes) and the time was 10:35 it would trigger at 10:40, 10:50, 11:00, and so on not "in ten minutes" at 10:45. This was done so that the clock would update properly when minutes changed.

## OTA uploads

There's a new menu item "Update OTA" that puts the Watchy into "OTA update" mode. You can then upload new software using `espota.py` See the ESP32 OTA update documentation for more details.

## Refactoring

The BLE classes were refactored slightly for better encapsulation (moved globals into the class) and the accelerometer "sensor" was moved into it's own class.
