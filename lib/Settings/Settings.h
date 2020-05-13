#include <Arduino.h>
#include <ESP_EEPROM.h>
#include <Log.h>

struct PersistedSettings {
  uint8_t pattern;
  uint32_t startColor;
  uint32_t endColor;
  uint8_t brightness;
  uint8_t speed;
};

class Settings {
 public:
  PersistedSettings init();
  bool commit(PersistedSettings);
};