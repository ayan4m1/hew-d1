#include "Light.h"

Light::Light(uint8_t ledCount, uint8_t ledPin) {
  config = LightConfig();
  config.ledCount = ledCount;
  config.ledPin = ledPin;

  Log::log("Configuring %d LEDs on pin %d", ledCount, ledPin);
  pixels = new CRGB[ledCount];
  FastLED.addLeds<WS2812B, D4, GRB>(pixels, ledCount)
      .setCorrection(TypicalLEDStrip);
}

/**
 * Start communicating with LEDs and turn them off.
 */
void Light::init(uint8_t brightness, uint32_t color) {
  // turn off built-in LED
  pinMode(LED_BUILTIN, HIGH);

  update(brightness, color);
}

/**
 * Set all pixels to the saved brightness/color values.
 */
void Light::update(uint8_t brightness, uint32_t color) {
  Log::log("Setting brightness to %d", brightness);
  FastLED.setBrightness(brightness);

  CRGB parsed = CRGB(color);
  Log::log("Setting color to (%d, %d, %d)", parsed.r, parsed.g, parsed.b);
  for (uint8_t i = 0; i < config.ledCount; i++) {
    pixels[i] = CRGB(parsed);
  }

  FastLED.show();
}

uint32_t Light::getColor(uint8_t r, uint8_t g, uint8_t b) {
  return (r << 16) + (g << 8) + b;
}