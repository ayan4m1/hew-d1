#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WebServer.h>

// Hardcoded HTTP request/response strings
#define RESPONSE_OK F("HTTP/1.1 200 OK")
#define RESPONSE_BAD_REQUEST F("HTTP/1.1 400 Bad Request")
#define RESPONSE_INTERNAL_SERVER_ERROR F("HTTP/1.1 500 Internal Server Error")

#define HEADER_ACCESS_CONTROL_ALLOW_ORIGIN F("Access-Control-Allow-Origin: *")
#define HEADER_ACCESS_CONTROL_ALLOW_HEADERS F("Access-Control-Allow-Headers: content-type")
#define HEADER_CONNECTION F("Connection: close")
#define HEADER_CONTENT_TYPE_JSON F("Content-type: application/json")

// Calculated using https://arduinojson.org/v6/assistant/
#define JSON_PACKET_SIZE JSON_OBJECT_SIZE(4) + 10

struct WebConfig {
  uint16_t port;
  uint32_t timeoutMs;
};

struct WebResponse {
  uint8_t brightness;
  uint8_t red;
  uint8_t green;
  uint8_t blue;
};

class Web {
  WebConfig config;
  WiFiServer* server;

 public:
  Web(uint16_t, uint32_t);
  void init();
  bool poll(WebResponse*);
};
