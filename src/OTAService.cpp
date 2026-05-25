#include "OTAService.h"

const char* FirmwareUpdateService::_otaDeviceKey = nullptr;

static const char certPem[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

FirmwareUpdateService::FirmwareUpdateService(const char* firmhubHost,
                                             const char* deviceKey,
                                             const char* nvsNamespace,
                                             const char* nvsKeyVersion,
                                             unsigned long heartbeatMs,
                                             unsigned long otaPollMs,
                                             unsigned long callbackIntervalMs,
                                             HardwareSerial& debugSerial)
  : _firmhubHost(firmhubHost),
    _deviceKey(deviceKey),
    _nvsNamespace(nvsNamespace),
    _nvsKeyVersion(nvsKeyVersion),
    _heartbeatMs(heartbeatMs),
    _otaPollMs(otaPollMs),
    _callbackIntervalMs(callbackIntervalMs),
    _version(""),
    _lastHeartbeat(0),
    _lastOtaCheck(0),
    _otaDownloaded(0),
    _otaTotal(0),
    _progressCallback(nullptr),
    _lastCallbackMs(0),
    _debugSerial(debugSerial)
{
}

void FirmwareUpdateService::begin() {
  _version = readVersionOnBoot();
  _debugSerial.print("\n\n[BOOT] Starting device...");
  _debugSerial.println(_version);
}

void FirmwareUpdateService::handleLoop() {
  unsigned long now = millis();
  if (now - _lastHeartbeat >= _heartbeatMs) {
    sendHeartbeat();
    _lastHeartbeat = now;
  }
  if (now - _lastOtaCheck >= _otaPollMs) {
    checkOta();
    _lastOtaCheck = now;
  }
  if (_isResetRequire && (now - _lastResetMs >= _resetDelayMs)) {
    _debugSerial.println("[RESET] Restarting device...");
    ESP.restart();
  }
}

void FirmwareUpdateService::setOtaProgressCallback(OtaProgressCallback cb) {
  _progressCallback = cb;
}

String FirmwareUpdateService::currentVersion() const {
  return _version;
}

String FirmwareUpdateService::readVersionOnBoot() {
  _prefs.begin(_nvsNamespace, true);
  String saved = _prefs.getString(_nvsKeyVersion, "");
  _prefs.end();

  if (saved.length() > 0) {
    _debugSerial.printf("[BOOT] version (NVS): %s\n", saved.c_str());
    return saved;
  }

  const esp_app_desc_t* desc = esp_ota_get_app_description();
  String fromBinary = String(desc->version);
  _debugSerial.printf("[BOOT] version (binary): %s\n", fromBinary.c_str());
  return fromBinary;
}

void FirmwareUpdateService::saveVersion(const String& ver) {
  _prefs.begin(_nvsNamespace, false);
  _prefs.putString(_nvsKeyVersion, ver);
  _prefs.end();
  _debugSerial.printf("[VER] saved → %s\n", ver.c_str());
}

void FirmwareUpdateService::sendHeartbeat() {
  _debugSerial.println("[HB] sending heartbeat...");
  HTTPClient http;
  http.begin(String(_firmhubHost) + "/device/v1/heartbeat");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-Device-Key", _deviceKey);

  StaticJsonDocument<256> doc;
  doc["currentVersion"]  = _version;
  doc["freeRamBytes"]    = (long)esp_get_free_heap_size();
  doc["uptimeSeconds"]   = millis() / 1000;
  doc["chipTempCelsius"] = temperatureRead();
  doc["wifiRssi"]        = WiFi.RSSI();

  String body;
  serializeJson(doc, body);
  int code = http.POST(body);
  if (code != 200) {
    _debugSerial.printf("[HB] http %d\n", code);
  }
  http.end();
}

String FirmwareUpdateService::readConfig() {
  HTTPClient http;
  http.begin(String(_firmhubHost) + "/device/v1/config");
  http.addHeader("X-Device-Key", _deviceKey);
  int code = http.GET();
  if (code != 200) {
    _debugSerial.printf("[CONFIG] http %d\n", code);
    http.end();
    return "";
  }

  String response = http.getString();
  http.end();
  _debugSerial.printf("[CONFIG] response: %s\n", response.c_str());
  return response;
}

void FirmwareUpdateService::reportOtaResult(bool success, const String& ver) {
  HTTPClient http;
  http.begin(String(_firmhubHost) + "/device/v1/ota/result");
  http.addHeader("Content-Type", "application/json");
  http.addHeader("X-Device-Key", _deviceKey);

  StaticJsonDocument<256> doc;
  doc["success"] = success;
  doc["version"] = ver;

  String body;
  serializeJson(doc, body);
  http.POST(body);
  _debugSerial.println("[OTA] reporting OTA result...");
  http.end();
}

void FirmwareUpdateService::checkOta() {
  _debugSerial.println("[OTA] checking for update...");
  HTTPClient http;
  http.begin(String(_firmhubHost) + "/device/v1/ota/check");
  http.addHeader("X-Device-Key", _deviceKey);
  http.addHeader("X-Current-Version", _version);

  int code = http.GET();
  _debugSerial.print("[OTA] http code: ");
  _debugSerial.println(code);
  if (code != 200) {
    _debugSerial.printf("[OTA] check failed: %d\n", code);
    http.end();
    return;
  }

  String response = http.getString();
  http.end();

  StaticJsonDocument<1024> res;
  DeserializationError error = deserializeJson(res, response);
  if (error) {
    _debugSerial.printf("[OTA] parse failed: %s\n", error.c_str());
    return;
  }

  _debugSerial.printf("[OTA] response body: %s\n", response.c_str());
  if (!res["hasUpdate"].as<bool>()) {
    return;
  }

  String newVer = res["version"].as<String>();
  String url = res["downloadUrl"].as<String>();
  _debugSerial.printf("[OTA] update available: %s\n", newVer.c_str());
  _debugSerial.printf("[OTA] download URL: %s\n", url.c_str());

  if (newVer == _version) {
    _debugSerial.println("[OTA] version on server is the same as current version, skipping update");
    return;
  }

  if (res.containsKey("sha256")) {
    String expectedHash = res["sha256"].as<String>();
    _debugSerial.printf("[OTA] expected SHA256: %s\n", expectedHash.c_str());
  }

  _otaDownloaded = 0;
  _otaTotal = 0;

  esp_http_client_config_t httpCfg = {};
  httpCfg.url = url.c_str();
  httpCfg.event_handler = _httpEventHandler;
  httpCfg.cert_pem = certPem;
  httpCfg.transport_type = HTTP_TRANSPORT_OVER_SSL;
  httpCfg.user_data = this;
  _otaDeviceKey = _deviceKey;

  esp_err_t err = esp_https_ota(&httpCfg);
  _otaDeviceKey = nullptr;

  if (err == ESP_OK) {
    saveVersion(newVer);
    reportOtaResult(true, newVer);
    _debugSerial.println("[OTA] success, restarting...");
    _isResetRequire = true;
    _lastResetMs = millis();
  } else {
    _debugSerial.printf("[OTA] flash error: %d\n", err);
    reportOtaResult(false, newVer);
    _debugSerial.println("[OTA] failed");
  }
}

esp_err_t FirmwareUpdateService::_httpEventHandler(esp_http_client_event_t* evt) {
  FirmwareUpdateService* self = static_cast<FirmwareUpdateService*>(evt->user_data);
  if (evt->event_id == HTTP_EVENT_ON_CONNECTED && _otaDeviceKey != nullptr) {
    esp_http_client_set_header(evt->client, "X-Device-Key", _otaDeviceKey);
  }

  if (self == nullptr) {
    return ESP_OK;
  }

  if (evt->event_id == HTTP_EVENT_ON_HEADER && evt->header_key && evt->header_value) {
    if (strcmp(evt->header_key, "Content-Length") == 0) {
      self->_otaTotal = atoi(evt->header_value);
    }
  }

  if (evt->event_id == HTTP_EVENT_ON_DATA) {
    self->_otaDownloaded += evt->data_len;
    if (self->_progressCallback) {
      self->_lastCallbackMs = millis();
      if (millis() - self->_lastCallbackMs >= self->_callbackIntervalMs) {
        int percent = -1;
        if (self->_otaTotal > 0) {
          percent = (int)((long long)self->_otaDownloaded * 100 / self->_otaTotal);
        }
        self->_progressCallback(percent, self->_otaDownloaded, self->_otaTotal);
      }
    }
  }

  return ESP_OK;
}
