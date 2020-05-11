#include <FastLED.h>
#include <Log.h>

enum Pattern : uint8_t {
  Solid = 1,
  Marquee = 2
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
  MarqueePattern(Light* light) : IPattern(light) {}

  void draw();

 private:
  bool direction = false;
  uint8_t lastValue = 127;
  uint8_t lastHue = 0;
  uint8_t lastIndex = 0;
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
  uint32_t getColor(uint8_t r, uint8_t g, uint8_t b);
  void setColor(uint8_t i, CRGB color);
};