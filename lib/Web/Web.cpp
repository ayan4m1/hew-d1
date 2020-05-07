#include "Web.h"

Web::Web(uint16_t port, uint32_t timeoutMs, String passphrase) {
  config = WebConfig();
  config.port = port;
  config.timeoutMs = timeoutMs;
  config.passphrase = passphrase;

  server = new WiFiServer(config.port);
}

void Web::init() {
  server->begin();
}

void Web::respond(String status, WiFiClient client) {
  client.println(status);
  client.println(HEADER_ACCESS_CONTROL_ALLOW_ORIGIN);
  client.println(HEADER_ACCESS_CONTROL_ALLOW_HEADERS);
  client.println(HEADER_CONNECTION);
  client.println();
  client.println();

  if (client.connected()) {
    client.stop();
  }
}

void Web::respond(String status, String response, WiFiClient client) {
  client.println(status);
  client.println(HEADER_ACCESS_CONTROL_ALLOW_ORIGIN);
  client.println(HEADER_ACCESS_CONTROL_ALLOW_HEADERS);
  client.println(HEADER_CONNECTION);
  client.println();
  client.println(response);
  client.println();

  if (client.connected()) {
    client.stop();
  }
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
    this->respond(RESPONSE_OK, client);
    return false;
  } else if (!request.startsWith("POST")) {
    this->respond(RESPONSE_BAD_REQUEST, client);
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
    this->respond(RESPONSE_INTERNAL_SERVER_ERROR, error.c_str(), client);
    return false;
  }

  if (doc["p"] != config.passphrase) {
    this->respond(RESPONSE_UNAUTHORIZED, client);
    return false;
  }

  this->respond(RESPONSE_OK, client);

  response->brightness = doc["br"];
  response->red = doc["r"];
  response->green = doc["g"];
  response->blue = doc["b"];
  return true;
}