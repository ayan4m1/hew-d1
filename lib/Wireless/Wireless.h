#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <Log.h>

#define MDNS_SERVICE "http"
#define MDNS_PROTOCOL "tcp"

struct WirelessConfig {
  String deviceIdentifier;
  uint16_t httpPort;
  String ssid;
  String passphrase;
};

class Wireless {
  WirelessConfig config;

 public:
  Wireless(String, uint16_t, String, String);
  ~Wireless();
  void poll();

 private:
  bool connect();
};
