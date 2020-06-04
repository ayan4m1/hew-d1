#include "Light.h"

uint8_t getHue(CRGB color) {
  uint8_t minValue = min(color.r, min(color.g, color.b));
  uint8_t maxValue = max(color.r, max(color.g, color.b));
  uint8_t delta = maxValue - minValue;

  if (maxValue == color.r) {
    return 43 * (color.g - color.b / (double)delta);
  } else if (maxValue == color.g) {
    return 43 * (2 + (color.b - color.r) / (double)delta);
  } else if (maxValue == color.b) {
    return 43 * (4 + (color.r - color.g) / (double)delta);
  }

  return 0;
}

void IPattern::draw() {}

void SolidPattern::draw() {
  Log::log("Setting color to (%d, %d, %d)", color.r, color.g, color.b);
  light->setColor(CRGB(color));
}

void MarqueePattern::draw() {
  Log::log("Marquee hue is %d", lastHue);
  light->setColor(lastIndex, CHSV(lastHue, 255, 200));
  delay(40 / (speed / (double)128U));

  if (!direction) {
    lastHue++;
  } else {
    lastHue--;
  }

  lastIndex++;

  if (lastIndex == light->config.ledCount) {
    lastIndex = 0;
  }

  if (lastHue == minHue || lastHue == maxHue) {
    direction = !direction;
  }
}

void GradientPattern::draw() {
  Log::log("Gradient step is %d/%d, direction %s", currentStep, maximumSteps, direction ? "true" : "false");
  uint8_t easedStep = ease8InOutCubic((currentStep / (double)maximumSteps) * 0xFFU);
  CRGB blendedColor = blend(startColor, endColor, easedStep);
  light->setColor(blendedColor);

  if (!direction) {
    currentStep++;
  } else {
    currentStep--;
  }

  if (currentStep == maximumSteps || currentStep == 0) {
    direction = !direction;
  }
}

Light::Light() {
  config = LightConfig();
  config.ledCount = HEW_LED_COUNT;
  config.ledPin = HEW_LED_PIN;
  config.ledBrightness = 0;

  Log::log("Configuring %d LEDs on pin %d", config.ledCount, config.ledPin);
  pixels = new CRGB[HEW_LED_COUNT];
  FastLED.addLeds<WS2812B, HEW_LED_PIN, GRB>(pixels, HEW_LED_COUNT);
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