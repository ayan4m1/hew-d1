#include "Settings.h"

PersistedSettings Settings::init() {
  auto config = PersistedSettings();
  EEPROM.begin(sizeof(PersistedSettings));

  if (EEPROM.percentUsed() >= 0) {
    Log::log("Loading device settings from EEPROM");
    EEPROM.get(0, config);
  } else {
    config.pattern = 1;
    config.speed = 1;
    config.startColor = 0;
    config.endColor = 0;
    config.brightness = 0;

    Log::log("Writing new device settings to EEPROM");
    EEPROM.put(0, config);
    EEPROM.commit();
  }

  EEPROM.end();
  return config;
}

bool Settings::commit(PersistedSettings config) {
  Log::log("Writing new device settings to EEPROM");

  EEPROM.begin(sizeof(PersistedSettings));
  EEPROM.put(0, config);
  bool result = EEPROM.commit();
  EEPROM.end();

  return result;
}