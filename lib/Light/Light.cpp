#include "Light.h"

void IPattern::draw() {}

void SolidPattern::draw() {
  // Log::log("Setting color to (%d, %d, %d)", color.r, color.g, color.b);

  for (uint8_t i = 0; i < light->config.ledCount; i++) {
    light->setColor(i, CRGB(color));
  }
}

void MarqueePattern::draw() {
  Log::log("Marquee hue is %d, value is %d", lastHue);
  light->setColor(lastIndex, CHSV(lastHue, 255, 200));

  if (direction) {
    lastHue -= 4;
  } else {
    lastHue += 4;
  }

  lastIndex++;

  if (lastIndex == light->config.ledCount) {
    lastIndex = 0;
  }

  if (lastHue == 0 || lastHue == 0xFF) {
    direction = !direction;
  }
}

Light::Light(uint8_t ledCount, uint8_t ledPin) {
  config = LightConfig();
  config.ledCount = ledCount;
  config.ledPin = ledPin;
  config.ledBrightness = 0;

  Log::log("Configuring %d LEDs on pin %d", ledCount, ledPin);
  pixels = new CRGB[ledCount];
  FastLED.addLeds<WS2812B, D4, GRB>(pixels, ledCount);
}

void Light::show() {
  if (config.pattern != NULL) {
    config.pattern->draw();
  }
  FastLED.show();
}

void Light::changeBrightness(uint8_t brightness) {
  Log::log("Setting brightness to %d", config.ledBrightness);
  config.ledBrightness = brightness;
  FastLED.setBrightness(config.ledBrightness);
}

void Light::changePattern(IPattern* pattern) {
  Log::log("Changing pattern");
  config.pattern = pattern;
}

void Light::setColor(uint8_t i, CRGB color) {
  pixels[i] = color;
}

void Light::setColor(CRGB color) {
  fill_solid(pixels, config.ledCount, color);
}