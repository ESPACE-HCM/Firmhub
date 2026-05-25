#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Preferences.h>
#include <ArduinoJson.h>
#include "esp_https_ota.h"
#include "esp_ota_ops.h"

class FirmwareUpdateService {
public:
  using OtaProgressCallback = void (*)(int percent, int downloaded, int total);

  FirmwareUpdateService(const char* firmhubHost,
                        const char* deviceKey,
                        const char* nvsNamespace = "firmhub",
                        const char* nvsKeyVersion = "V1.0.0",
                        unsigned long heartbeatMs = 60000,
                        unsigned long otaPollMs = 60000,
                        unsigned long callbackIntervalMs = 1000,
                        HardwareSerial& debugSerial = Serial);

  void begin();
  void handleLoop();
  void setOtaProgressCallback(OtaProgressCallback cb);
  String currentVersion() const;
  String readConfig();
  void sendHeartbeat();
  void checkOta();

private:
  String readVersionOnBoot();
  void saveVersion(const String& ver);
  void reportOtaResult(bool success, const String& ver);

  const char* _firmhubHost;
  const char* _deviceKey;
  const char* _nvsNamespace;
  const char* _nvsKeyVersion;
  HardwareSerial& _debugSerial;
  bool _isResetRequire = false;
  unsigned long _heartbeatMs;
  unsigned long _otaPollMs;
  unsigned long _callbackIntervalMs;
  unsigned long _resetDelayMs = 2000;

  Preferences _prefs;
  String _version;
  unsigned long _lastHeartbeat;
  unsigned long _lastOtaCheck;
  unsigned long _lastCallbackMs;
  unsigned long _lastResetMs;

  int _otaDownloaded;
  int _otaTotal;
  OtaProgressCallback _progressCallback;

  static const char* _otaDeviceKey;
  static esp_err_t _httpEventHandler(esp_http_client_event_t* evt);
};
