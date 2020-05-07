#include "Web.h"

Web::Web(uint16_t port, uint32_t timeoutMs) {
  config = WebConfig();
  config.port = port;
  config.timeoutMs = timeoutMs;

  server = new WiFiServer(config.port);
}

void Web::init() {
  server->begin();
}

bool Web::poll(WebResponse* response) {
  WiFiClient client = server->available();

  if (!client) {
    return false;
  }

  uint32_t currentTime = millis();
  uint32_t lastTime = currentTime;

  String request;
  StaticJsonDocument<JSON_PACKET_SIZE> doc;

  while (!client.available() && (currentTime - lastTime) <= config.timeoutMs) {
    currentTime = millis();
    delay(20);
  }

  request = client.readStringUntil('\n');

  if (request.startsWith("OPTIONS")) {
    client.println(RESPONSE_OK);
    client.println(HEADER_ACCESS_CONTROL_ALLOW_ORIGIN);
    client.println(HEADER_ACCESS_CONTROL_ALLOW_HEADERS);
    client.println(HEADER_CONNECTION);
    client.println();
    client.println();
    client.stop();
    return false;
  } else if (!request.startsWith("POST")) {
    // ensure this is a POST request
    client.println(RESPONSE_BAD_REQUEST);
    client.println(HEADER_ACCESS_CONTROL_ALLOW_ORIGIN);
    client.println(HEADER_ACCESS_CONTROL_ALLOW_HEADERS);
    client.println(HEADER_CONNECTION);
    client.println();
    client.println();

    if (client.connected()) {
      client.stop();
    }

    return false;
  }

  // skip all the headers
  // note: this is vulerable to DOS attacks
  while (!request.startsWith("{") && (currentTime - lastTime) <= config.timeoutMs) {
    currentTime = millis();
    request = client.readStringUntil('\n');
  }

  // try to deserialize the JSON
  DeserializationError error = deserializeJson(doc, request);

  if (error) {
    client.println(RESPONSE_INTERNAL_SERVER_ERROR);
    client.println(HEADER_ACCESS_CONTROL_ALLOW_ORIGIN);
    client.println(HEADER_ACCESS_CONTROL_ALLOW_HEADERS);
    client.println(HEADER_CONNECTION);
    client.println();
    client.println(error.c_str());
    client.println();

    if (client.connected()) {
      client.stop();
    }

    return false;
  }

  client.println(RESPONSE_OK);
  client.println(HEADER_ACCESS_CONTROL_ALLOW_ORIGIN);
  client.println(HEADER_ACCESS_CONTROL_ALLOW_HEADERS);
  client.println(HEADER_CONTENT_TYPE_JSON);
  client.println(HEADER_CONNECTION);
  client.println();
  client.println();

  if (client.connected()) {
    client.stop();
  }

  response->brightness = doc["br"];
  response->red = doc["r"];
  response->green = doc["g"];
  response->blue = doc["b"];
  return true;
}