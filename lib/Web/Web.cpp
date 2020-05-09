#include "Web.h"

WebResponse::WebResponse() {
  red = blue = green = brightness = 0;
}

WebResponse::WebResponse(uint8_t brightness, uint8_t red, uint8_t green, uint8_t blue) {
  this->brightness = brightness;
  this->red = red;
  this->green = green;
  this->blue = blue;
}

Web::Web(uint16_t port, uint32_t timeoutMs, String passphrase) {
  config = WebConfig();
  config.port = port;
  config.timeoutMs = timeoutMs;
  config.passphrase = passphrase;

  server = new WiFiServer(config.port);
}

void Web::init() {
  Log::log("Starting HTTP server...");
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

  Log::log("Got an HTTP connection");

  uint32_t currentTime = millis();
  uint32_t lastTime = currentTime;

  String request;
  StaticJsonDocument<JSON_PACKET_SIZE> doc;

  while (!client.available() && (currentTime - lastTime) <= config.timeoutMs) {
    currentTime = millis();
    delay(20);
  }

  Log::log("Reading HTTP request");

  request = client.readStringUntil('\n');

  if (request.startsWith("OPTIONS")) {
    Log::log("Handling pre-flight request");
    this->respond(RESPONSE_OK, client);
    return false;
  } else if (!request.startsWith("POST")) {
    Log::log("Got a non-POST request!");
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
    Log::log("JSON deserialization failed: %s", error.c_str());
    this->respond(RESPONSE_INTERNAL_SERVER_ERROR, error.c_str(), client);
    return false;
  }

  if (doc["p"] != config.passphrase) {
    Log::log("Invalid passphrase supplied!");
    this->respond(RESPONSE_UNAUTHORIZED, client);
    return false;
  }

  this->respond(RESPONSE_OK, client);

  *response = WebResponse(doc["br"], doc["r"], doc["g"], doc["b"]);

  Log::log("Got new device settings: (%d, %d, %d) %d",
           response->red,
           response->green,
           response->blue,
           response->brightness);

  return true;
}