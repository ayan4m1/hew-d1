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
#ifdef HEW_LOGGING
  Log::init(HEW_LOGGING_BAUD_RATE);
#endif

  // turn off built-in LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  // initialize objects with configuration
  light = new Light(HEW_LED_COUNT, HEW_LED_PIN);
  settings = new Settings();
  wireless = new Wireless(
      HEW_DEVICE_IDENTIFIER,
      HEW_HTTP_PORT,
      HEW_WIFI_SSID,
      HEW_WIFI_PSK);
  api = new Web(HEW_HTTP_PORT, HEW_HTTP_TIMEOUT_MS, HEW_DEVICE_PASSPHRASE);

  // load settings from NVRAM
  deviceSettings = settings->init();

  // init light state based on settings
  light->changeBrightness(deviceSettings.brightness);
  if (deviceSettings.pattern == Solid) {
    light->changePattern(new SolidPattern(light, deviceSettings.color));
  } else if (deviceSettings.pattern == Marquee) {
    light->changePattern(new MarqueePattern(light));
  }
  light->show();

  // start listening over HTTP
  api->init();
}

void loop() {
  // update wireless state
  wireless->poll();

  WebResponse response = WebResponse();
  if (api->poll(&response)) {
    const uint32_t newColor = light->getColor(response.red, response.green, response.blue);
    const uint8_t newBrightness = response.brightness;
    const String newPattern = response.pattern;
    boolean settingsChanged = false;

    if (deviceSettings.brightness != newBrightness) {
      deviceSettings.brightness = newBrightness;
      light->changeBrightness(newBrightness);
      settingsChanged = true;
    }

    if (deviceSettings.color != newColor) {
      deviceSettings.color = newColor;
      settingsChanged = true;
    }

    if (deviceSettings.pattern != Solid && newPattern == "SOLID") {
      deviceSettings.pattern = Solid;
      light->changePattern(new SolidPattern(light, deviceSettings.color));
      settingsChanged = true;
    } else if (deviceSettings.pattern != Marquee && newPattern == "MARQUEE") {
      deviceSettings.pattern = Marquee;
      light->changePattern(new MarqueePattern(light));
      settingsChanged = true;
    }

    // update pixels and save new settings
    if (settingsChanged) {
      Log::log("Settings have changed");
      settings->commit(deviceSettings);
    }
  }

  // update light state
  light->show();
  delay(50);
}