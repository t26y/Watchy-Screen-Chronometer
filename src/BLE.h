#pragma once

#include "Arduino.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include "esp_ota_ops.h"

#include "config.h"

class BLE
{
 private:
  String local_name;

  BLEServer* pServer = NULL;

  BLEService* pESPOTAService = NULL;
  BLECharacteristic* pESPOTAIdCharacteristic = NULL;

  BLEService* pService = NULL;
  BLECharacteristic* pVersionCharacteristic = NULL;
  BLECharacteristic* pOtaCharacteristic = NULL;
  BLECharacteristic* pWatchFaceNameCharacteristic = NULL;

 public:
  BLE(){};

  int status = -1;
  int bytesReceived = 0;
  bool updateFlag = false;

  void begin(const char* localName = "Watchy BLE OTA");
  int getStatus() { return status; };
  int getBytesReceived() { return bytesReceived; };
};