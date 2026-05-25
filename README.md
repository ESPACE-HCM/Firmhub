# OTA_Payment

Firmhub ESP32 OTA firmware update and device management library.

## Overview

`FirmwareUpdateService` provides a complete ESP32 device lifecycle integration for Firmhub:
- secure HTTPS OTA firmware updates via `esp_https_ota`
- periodic heartbeat reporting to the Firmhub backend
- remote configuration fetch from the device config endpoint
- OTA result reporting after update attempts
- persistent version tracking using NVS

## Features

- Automatic OTA polling and download
- Config refresh support with `readConfig()`
- Progress callback support for OTA download status
- Compatible with Arduino and PlatformIO projects
- Designed for ESP32-based devices

## Installation

### Arduino IDE

1. Copy the `Firmhub` folder into your Arduino `libraries` directory.
2. Restart the Arduino IDE.
3. Add `#include <OTAService.h>` to your sketch.

### PlatformIO

Add this library as a dependency in `platformio.ini`:

```ini
lib_deps = https://github.com/yourname/Firmhub.git
```

Or if you keep the library locally:

```ini
lib_extra_dirs = /path/to/Firmhub
```

## Usage

```cpp
#include <WiFi.h>
#include <OTAService.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const char* firmhubHost = "https://your-firmhub-server.com";
const char* deviceKey = "YOUR_DEVICE_KEY";

FirmwareUpdateService otaService(firmhubHost, deviceKey);

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  otaService.begin();
}

void loop() {
  otaService.handleLoop();
}
```

## Configuration Fetch

Use `readConfig()` to request the latest device configuration from Firmhub.
The method returns the raw JSON response as a `String`.

Example:

```cpp
String config = otaService.readConfig();
if (config.length() > 0) {
  Serial.println("Config received:");
  Serial.println(config);
} else {
  Serial.println("No config received or request failed.");
}
```

Call `readConfig()` periodically from `loop()` to keep device settings up to date.

## Dependencies

- ArduinoJson
- WiFi
- HTTPClient
- Preferences
- esp_https_ota

## Example

See `examples/OTA_Payment/OTA_Payment.ino` for a complete working example including OTA and config polling.
