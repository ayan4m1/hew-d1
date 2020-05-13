#include <FastLED.h>
#include <Log.h>

uint8_t getHue(CRGB color);

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
    minHue = min(getHue(startColor), getHue(endColor));
    maxHue = max(getHue(startColor), getHue(endColor));
    lastHue = minHue;
    Log::log("Min hue %d max hue %d", minHue, maxHue);
    this->speed = speed;
  }

  void draw();

 private:
  bool direction = false;
  uint8_t minHue = 0;
  uint8_t maxHue = 0xFFU;
  uint8_t speed = 127;
  uint8_t lastHue = 0;
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

  Light(uint8_t ledCount, uint8_t ledPin);

  void init(uint8_t brightness, uint32_t color);
  void changeBrightness(uint8_t brightness);
  void changePattern(IPattern* pattern);
  void show();
  void setColor(uint8_t i, CRGB color);
  void setColor(CRGB color);
};