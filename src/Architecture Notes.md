# Architecture Notes

## Screens

* Buzz  
  Runs for a fixed time with no display, then returns to parent
* Carousel  
  Waits for a button press event (up, down, menu, back)
* Get Weather  
  Updates weather data in asynchronously in background, foreground loops, back returns to parent
* Show Weather  
  passive display with default back button
* Icon  
  passive display with default back button
* Image  
  passive display with default back button
* Menu  
  Waits for a button press event (up, down, menu, back)
* Set Location  
  Updates location data in asynchronously in background, foreground loops, back returns to parent
* Set Time  
  Updates time synchronously in foreground, then returns to parent (uses buttons)
* Setup Wifi  
  Sets wifi via captive portal asynchronously in background. foreground loops waiting for success or timeout.
* Battery  
  passive display with default back button
* Bluetooth Status  
  passive display with default back button
* Orientation  
  actively updates accelerometer display in foreground loop until dismissed by back button
* Steps  
  passive display with default back button
* Wifi Status  
  passive display with default back button
* Sync Time  
  Updates time data in asynchronously in background, foreground loops, back returns to parent
* Time  
  periodic update, default back button
* Update Firmware  
  BLE OTA firmware update, confirm via menu btn, synchronous, non-cancellable, default back
* Weather  
  passive display with default back button 
* Wrapped Text  
  passive display with default back button

## Styles

* Passive display with default back button
* No display, run for fixed time, return to parent
* Consume some number of buttons for UI (menu, up, down, back)
  * does not block, returns while waiting for next button event
  * blocks, loops while waiting for next button event (because it blinks)
* perform a (network based) task in the background. foreground waits for it to complete, or time out. displays status till dismissed (with back button)
* continuously display changes in sensor in foreground (accelerometer)
* perform a (BLE based) task in the background. foreground blocks displaying status
* display updates periodically

## Architecture

Deep sleep except when performing background task (OTA update, network time [NTP UDP], network location [HTTP GET], network weather [HTTP GET])

* Periodic updates (1 min default, 1 sec for blink, 200 ms for accel?)
* Button interrupts
* wait for background wifi task
* wait for background ble task

Have a "stay awake" timer that gets set to 5 seconds every time there's a user interaction. Buttons for sure, but also other interrupts like accelerometer? Don't sleep until the timer expires.

Don't wake up every minute unless we have to.

Screens register when their `show()` method should get called (in the constructor?) By default, for compatibility, they get called on everything.

## Future

Spawn a background task then return. Keep the UI active, possibly spawning additional background tasks. Tasks can share WIFI resource, refcounted. Don't sleep until all tasks are completed or timed out. Release Wifi resource with refcount 0.
