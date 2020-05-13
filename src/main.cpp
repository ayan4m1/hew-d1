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
    light->changePattern(new SolidPattern(light, deviceSettings.startColor));
  } else if (deviceSettings.pattern == Gradient) {
    light->changePattern(new GradientPattern(light, deviceSettings.startColor, deviceSettings.endColor, deviceSettings.speed));
  } else if (deviceSettings.pattern == Marquee) {
    light->changePattern(new MarqueePattern(light, deviceSettings.startColor, deviceSettings.endColor, deviceSettings.speed));
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
    boolean settingsChanged = false;

    if (deviceSettings.brightness != response.brightness) {
      deviceSettings.brightness = response.brightness;
      light->changeBrightness(response.brightness);
      settingsChanged = true;
    }

    if (deviceSettings.startColor != response.startColor) {
      deviceSettings.startColor = response.startColor;
      settingsChanged = true;
    }

    if (deviceSettings.endColor != response.endColor) {
      deviceSettings.endColor = response.endColor;
      settingsChanged = true;
    }

    if (deviceSettings.speed != response.speed) {
      deviceSettings.speed = response.speed;
      settingsChanged = true;
    }

    if (response.pattern == "SOLID") {
      deviceSettings.pattern = Solid;
      light->changePattern(new SolidPattern(light, deviceSettings.startColor));
      settingsChanged = true;
    } else if (response.pattern == "GRADIENT") {
      deviceSettings.pattern = Gradient;
      light->changePattern(new GradientPattern(light, deviceSettings.startColor, deviceSettings.endColor, deviceSettings.speed));
      settingsChanged = true;
    } else if (response.pattern == "MARQUEE") {
      deviceSettings.pattern = Marquee;
      light->changePattern(new MarqueePattern(light, deviceSettings.startColor, deviceSettings.endColor, deviceSettings.speed));
      settingsChanged = true;
    }

    // update pixels and save new settings
    if (settingsChanged) {
      Log::log("Settings have changed");
      settings->commit(deviceSettings);
    }
  }

  // update light state at 120Hz
  EVERY_N_MILLISECONDS(8) {
    light->show();
  }
}