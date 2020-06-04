#include "Web.h"

WebResponse::WebResponse() {
  pattern = "";
  startColor = endColor = brightness = 0;
}

WebResponse::WebResponse(String pattern, uint8_t brightness, String startColor, String endColor, uint8_t speed) {
  this->pattern = pattern;
  this->brightness = brightness;
  this->startColor = strtoul(startColor.c_str(), 0, 16);
  this->endColor = strtoul(endColor.c_str(), 0, 16);
  this->speed = speed;
}

Web::Web() {
  config = WebConfig();
  config.port = HEW_HTTP_PORT;
  config.timeoutMs = HEW_HTTP_TIMEOUT_MS;
  config.passphrase = HEW_DEVICE_PASSPHRASE;

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

    // check for valid auth header
    if (request.startsWith("Authorization: ")) {
      if (!request.startsWith("Authorization: " + config.passphrase)) {
        Log::log("Invalid passphrase supplied!");
        this->respond(RESPONSE_UNAUTHORIZED, client);
        return false;
      }
    }

    request = client.readStringUntil('\n');
  }

  // try to deserialize the JSON
  DeserializationError error = deserializeJson(doc, request);

  if (error) {
    Log::log("JSON deserialization failed: %s", error.c_str());
    this->respond(RESPONSE_INTERNAL_SERVER_ERROR, error.c_str(), client);
    return false;
  }

  this->respond(RESPONSE_OK, client);

  *response = WebResponse(
      doc["pattern"],
      doc["brightness"],
      doc["startColor"],
      doc["endColor"],
      doc["speed"]);

  Log::log("Pattern: %s Colors: %d to %d Brightness: %d",
           response->pattern.c_str(),
           response->startColor,
           response->endColor,
           response->brightness);

  return true;
}