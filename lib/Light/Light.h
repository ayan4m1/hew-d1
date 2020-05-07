#include <Adafruit_NeoPixel.h>

struct LightConfig {
  uint8_t ledCount;
  uint8_t ledPin;
};

class Light {
  Adafruit_NeoPixel* pixels;
  LightConfig config;

 public:
  Light(uint8_t, uint8_t);
  void init();
  void update(uint8_t, uint32_t);
  uint32_t getColor(uint8_t r, uint8_t g, uint8_t b);
};