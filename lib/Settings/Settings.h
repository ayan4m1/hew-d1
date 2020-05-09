#include <Arduino.h>
#include <ESP_EEPROM.h>
#include <Log.h>

struct PersistedSettings {
  uint32_t color;
  uint8_t brightness;
};

class Settings {
 public:
  PersistedSettings init();
  bool commit(PersistedSettings);
};