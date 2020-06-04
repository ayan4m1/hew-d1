#include <Config.h>
#include <FastLED.h>
#include <Log.h>

CHSV getHSV(CRGB color);

enum Pattern : uint8_t {
  Solid = 1,
  Marquee = 2,
  Gradient = 4
};

class Light;

struct IPattern {
  Light* light;

  IPattern(Light* light) {
    this->light = light;
  }

  virtual ~IPattern() = default;
  virtual void draw();
};

struct SolidPattern : IPattern {
  SolidPattern(Light* light, uint32_t color) : IPattern(light) {
    this->color = CRGB(color);
  }

  void draw();

 private:
  CRGB color;
};

struct MarqueePattern : IPattern {
  MarqueePattern(Light* light, uint32_t startColor, uint32_t endColor, uint8_t speed) : IPattern(light) {
    CHSV startHsv = getHSV(startColor);
    CHSV endHsv = getHSV(endColor);
    minHue = min(startHsv.h, endHsv.h);
    maxHue = max(startHsv.h, endHsv.h);
    minSat = min(startHsv.s, endHsv.s);
    maxSat = max(startHsv.s, endHsv.s);
    minVal = min(startHsv.v, endHsv.v);
    maxVal = max(startHsv.v, endHsv.v);
    lastHue = min(255, minHue + 1);
    lastSat = min(255, minSat + 1);
    lastVal = min(255, minVal + 1);
    Log::log("Min hue %d max hue %d", minHue, maxHue);
    Log::log("Min sat %d max sat %d", minSat, maxSat);
    Log::log("Min val %d max val %d", minVal, maxVal);
    this->speed = speed;
  }

  void draw();

 private:
  bool hueDirection = false, satDirection = false, valDirection = false;
  uint8_t minHue = 0, minSat = 0, minVal = 0;
  uint8_t maxHue = 0xFFU, maxSat = 0xFFU, maxVal = 0xFFU;
  uint8_t speed = 127;
  uint8_t lastHue = 0;
  uint8_t lastSat = 0;
  uint8_t lastVal = 0;
  uint8_t lastIndex = 0;
};

struct GradientPattern : IPattern {
  GradientPattern(Light* light, uint32_t startColor, uint32_t endColor, uint8_t speed) : IPattern(light) {
    this->startColor = CRGB(startColor);
    this->endColor = CRGB(endColor);
    this->maximumSteps = 1024U * (128U / (double)speed);
  }

  void draw();

 private:
  bool direction = false;
  CRGB startColor;
  CRGB endColor;
  uint16_t maximumSteps = 0;
  uint16_t currentStep = 0;
};

struct LightConfig {
  uint8_t ledCount;
  uint8_t ledPin;
  uint8_t ledBrightness;
  IPattern* pattern;
};

struct Light {
  CRGB* pixels;
  LightConfig config;

  Light();

  void init(uint8_t brightness, uint32_t color);
  void changeBrightness(uint8_t brightness);
  void changePattern(IPattern* pattern);
  void show();
  void setColor(uint8_t i, CRGB color);
  void setColor(CRGB color);
};