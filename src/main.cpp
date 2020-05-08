#include <Arduino.h>
#include <Light.h>
#include <Settings.h>
#include <Web.h>
#include <Wireless.h>

#include "config.h"

Light* light;
Settings* settings;
Wireless* wireless;
Web* api;

PersistedSettings deviceSettings;

void setup() {
  light = new Light(HEW_LED_COUNT, HEW_LED_PIN);
  settings = new Settings();
  wireless = new Wireless(
      HEW_DEVICE_IDENTIFIER,
      HEW_HTTP_PORT,
      HEW_WIFI_SSID,
      HEW_WIFI_PSK);
  api = new Web(HEW_HTTP_PORT, HEW_HTTP_TIMEOUT_MS, HEW_DEVICE_PASSPHRASE);

  deviceSettings = settings->init();
  light->init(deviceSettings.brightness, deviceSettings.color);
  api->init();
}

void loop() {
  wireless->poll();

  WebResponse response = WebResponse();
  if (api->poll(&response)) {
    const uint32_t newColor = light->getColor(response.red, response.green, response.blue);
    const uint8_t newBrightness = response.brightness;
    boolean settingsChanged = false;

    // sync settings if they have changed
    if (deviceSettings.color != newColor) {
      deviceSettings.color = newColor;
      settingsChanged = true;
    }

    if (deviceSettings.brightness != newBrightness) {
      deviceSettings.brightness = newBrightness;
      settingsChanged = true;
    }

    // update pixels and save new settings
    if (settingsChanged) {
      light->update(deviceSettings.brightness, deviceSettings.color);
      settings->commit(deviceSettings);
    }
  }
}