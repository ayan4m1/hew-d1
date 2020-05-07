#include "Settings.h"

PersistedSettings Settings::init() {
  auto config = PersistedSettings();
  EEPROM.begin(sizeof(PersistedSettings));

  if (EEPROM.percentUsed() >= 0) {
    EEPROM.get(0, config);
  } else {
    config.color = 0;
    config.brightness = 0;

    EEPROM.put(0, config);
    EEPROM.commit();
  }

  EEPROM.end();
  return config;
}

bool Settings::commit(PersistedSettings config) {
  EEPROM.begin(sizeof(PersistedSettings));
  EEPROM.put(0, config);
  bool result = EEPROM.commit();
  EEPROM.end();

  return result;
}