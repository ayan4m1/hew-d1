#include "Light.h"

#define CHIPSET WS2812
#define DATA_PIN D4
#define COLOR_ORDER GRB

Light::Light(uint8_t ledCount, uint8_t ledPin) {
  config = LightConfig();
  config.ledCount = ledCount;
  config.ledPin = ledPin;

  pixels = new CRGB[ledCount];
  FastLED.addLeds<CHIPSET, DATA_PIN, COLOR_ORDER>(pixels, ledCount)
      .setCorrection(TypicalLEDStrip);
}

/**
 * Start communicating with LEDs and turn them off.
 */
void Light::init(uint8_t brightness, uint32_t color) {
  // turn off built-in LED
  pinMode(LED_BUILTIN, HIGH);

  FastLED.setBrightness(brightness);
  for (uint8_t i = 0; i < config.ledCount; i++) {
    pixels[i] = CRGB(color);
  }

  FastLED.show();
}

/**
 * Set all pixels to the saved brightness/color values.
 */
void Light::update(uint8_t brightness, uint32_t color) {
  FastLED.setBrightness(brightness);

  // note: this code does not support > 256 LEDs
  for (uint8_t i = 0; i < config.ledCount; i++) {
    pixels[i] = CRGB(color);
  }

  FastLED.show();
}

uint32_t Light::getColor(uint8_t r, uint8_t g, uint8_t b) {
  return CRGB(r, g, b);
}