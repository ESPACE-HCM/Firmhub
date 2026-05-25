# OTA_Payment

ESP32 OTA update and heartbeat service for Firmhub devices.

## Overview

This library provides `FirmwareUpdateService`, which handles:
- automatic HTTPS OTA firmware downloads on ESP32
- heartbeat reporting to the Firmhub server
- remote configuration fetch
- OTA result reporting

## Installation

### Arduino IDE

1. Copy the `Firmhub` folder into your Arduino `libraries` directory.
2. Restart the Arduino IDE.
3. Include the library with `#include <OTAService.h>`.

### PlatformIO

Add this library as a dependency in `platformio.ini`:

```ini
lib_deps = https://github.com/yourname/Firmhub.git
```

Or add the library local path:

```ini
lib_extra_dirs = /path/to/Firmhub
```

## Dependencies

- ArduinoJson
- WiFi
- HTTPClient
- Preferences
- esp_https_ota

## Example

See `examples/OTA_Payment/OTA_Payment.ino` for a minimal usage example.
