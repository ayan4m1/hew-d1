#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <Log.h>

// Hardcoded HTTP request/response strings
#define RESPONSE_OK F("HTTP/1.1 200 OK")
#define RESPONSE_BAD_REQUEST F("HTTP/1.1 400 Bad Request")
#define RESPONSE_UNAUTHORIZED F("HTTP/1.1 401 Unauthorized")
#define RESPONSE_INTERNAL_SERVER_ERROR F("HTTP/1.1 500 Internal Server Error")

#define HEADER_ACCESS_CONTROL_ALLOW_ORIGIN F("Access-Control-Allow-Origin: *")
#define HEADER_ACCESS_CONTROL_ALLOW_HEADERS F("Access-Control-Allow-Headers: content-type")
#define HEADER_CONNECTION F("Connection: close")
#define HEADER_CONTENT_TYPE_JSON F("Content-type: application/json")

// Calculated using https://arduinojson.org/v6/assistant/
/*
  {
    "pattern": "SOLID",
    "startColor": "ff0000",
    "endColor": "f0f0f0",
    "brightness": 255,
    "speed": 1,
    "passphrase": "12312312311231231231123123123112"
  }
 */
#define JSON_PACKET_SIZE JSON_OBJECT_SIZE(6) + 130

struct WebConfig {
  uint16_t port;
  uint32_t timeoutMs;
  String passphrase;
};

struct WebResponse {
  String pattern;
  uint8_t brightness;
  uint32_t startColor;
  uint32_t endColor;
  uint8_t speed;

  WebResponse();
  WebResponse(String pattern, uint8_t brightness, String startColor, String endColor, uint8_t speed);
};

class Web {
  WebConfig config;
  WiFiServer* server;

 public:
  Web(uint16_t, uint32_t, String);
  void init();
  bool poll(WebResponse*);

 private:
  void respond(String, WiFiClient);
  void respond(String, String, WiFiClient);
};
