#include "Wireless.h"

Wireless::Wireless() {
  config = WirelessConfig();
  config.deviceIdentifier = HEW_DEVICE_IDENTIFIER;
  config.httpPort = HEW_HTTP_PORT;
  config.ssid = HEW_WIFI_SSID;
  config.passphrase = HEW_WIFI_PSK;

  Log::log("Configuring WiFi for SSID %s", config.ssid.c_str());
  Log::log("Hostname is %s.local", config.deviceIdentifier.c_str());

  // set up WiFi radio
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.hostname(config.deviceIdentifier);
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

  Log::log("Attempting WiFi connection...");

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

  Log::log("IP address is %s", WiFi.localIP().toString().c_str());

  return WiFi.status() == WL_CONNECTED;
}

/**
 * If WiFi connection is interrupted, periodically attempt to
 * reconnect to WiFi. Blink the built-in LED during this process.
 */
void Wireless::poll() {
  if (!WiFi.isConnected()) {
    bool blink = LOW;
    while (1) {
      digitalWrite(LED_BUILTIN, blink);
      blink = !blink;

      WiFi.reconnect();
      if (this->connect()) {
        Log::log("Successfully connected to WiFi!");

        // set up MDNS
        if (!MDNS.begin(config.deviceIdentifier)) {
          Log::log("Error initializing mDNS!");
        }

        MDNS.addService(MDNS_SERVICE, MDNS_PROTOCOL, config.httpPort);

        digitalWrite(LED_BUILTIN, HIGH);
        break;
      }
    }
  }

  MDNS.update();
}
