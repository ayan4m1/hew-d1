#include "Light.h"

Light::Light(uint8_t ledCount, uint8_t ledPin) {
  config = LightConfig();
  config.ledCount = ledCount;
  config.ledPin = ledPin;

  pixels = new Adafruit_NeoPixel(config.ledCount, config.ledPin, NEO_GRB + NEO_KHZ800);
}

/**
 * Start communicating with LEDs and turn them off.
 */
void Light::init(uint8_t brightness, uint32_t color) {
  // turn off built-in LED
  pinMode(LED_BUILTIN, HIGH);

  pixels->begin();
  pixels->setBrightness(brightness);
  pixels->show();
  pixels->fill(color);
  pixels->show();
}

/**
 * Set all pixels to the saved brightness/color values.
 */
void Light::update(uint8_t brightness, uint32_t color) {
  pixels->setBrightness(brightness);

  // note: this code does not support > 256 LEDs
  for (uint8_t i = 0; i < this->config.ledCount; i++) {
    pixels->setPixelColor(i, color);
  }

  pixels->show();
}

uint32_t Light::getColor(uint8_t r, uint8_t g, uint8_t b) {
  return pixels->Color(r, g, b);
}