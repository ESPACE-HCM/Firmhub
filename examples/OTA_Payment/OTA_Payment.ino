#include <Arduino.h>
#include <WiFi.h>
#include <OTAService.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
const char* firmhubHost = "https://your-firmhub-server.com";
const char* deviceKey = "YOUR_DEVICE_KEY";

FirmwareUpdateService otaService(firmhubHost, deviceKey);

unsigned long lastConfigMillis = 0;
const unsigned long configIntervalMs = 30000;

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.println("\nWiFi connected");

  otaService.begin();
}

void loop() {
  otaService.handleLoop();

  unsigned long now = millis();
  if (now - lastConfigMillis >= configIntervalMs) {
    lastConfigMillis = now;
    String config = otaService.readConfig();
    if (config.length() > 0) {
      Serial.println("[CONFIG] Received config:");
      Serial.println(config);
    } else {
      Serial.println("[CONFIG] No config received or request failed.");
    }
  }

  delay(100);
}
