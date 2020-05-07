#include "Wireless.h"

Wireless::Wireless(String deviceIdentifier, uint16_t httpPort, String ssid, String passphrase) {
  config = WirelessConfig();
  config.deviceIdentifier = deviceIdentifier;
  config.httpPort = httpPort;
  config.ssid = ssid;
  config.passphrase = passphrase;

  // set up WiFi radio
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.hostname(config.deviceIdentifier);

  // set up MDNS
  MDNS.begin(config.deviceIdentifier);
  MDNS.addService(MDNS_SERVICE, MDNS_PROTOCOL, config.httpPort);
}

Wireless::~Wireless() {
  MDNS.end();

  if (WiFi.isConnected()) {
    WiFi.disconnect();
  }

  WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
}

/**
 * Allow up to ten seconds to connect to WiFi.
 */
bool Wireless::connect() {
  if (WiFi.isConnected()) {
    return true;
  }

  // give it ten seconds
  uint16_t timeoutMs = 10000;
  // half a second per tick
  const uint16_t delayMs = 500;

  WiFi.mode(WIFI_STA);
  WiFi.begin(config.ssid, config.passphrase);

  while (WiFi.status() != WL_CONNECTED) {
    delay(delayMs);
    timeoutMs -= delayMs;
    if (timeoutMs <= 0) {
      return false;
    }
  }

  return WiFi.status() == WL_CONNECTED;
}

/**
 * If WiFi connection is interrupted, periodically attempt to
 * reconnect to WiFi. Blink the built-in LED during this process.
 */
void Wireless::poll() {
  if (!WiFi.isConnected()) {
    bool blink = LOW;
    pinMode(LED_BUILTIN, OUTPUT);
    while (1) {
      digitalWrite(LED_BUILTIN, blink);
      blink = !blink;

      WiFi.reconnect();
      if (this->connect()) {
        digitalWrite(LED_BUILTIN, LOW);
        break;
      }
    }
  }

  MDNS.update();
}
