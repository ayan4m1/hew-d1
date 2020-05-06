#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP_EEPROM.h>

// Change this to your Wireless SSID
#define WIFI_AP "SSID"
// Change this to your WPA2 PSK
#define WIFI_PSK "ChangeMe"

// Ensure that each device on your network has a unique identifier
#define DEVICE_IDENTIFIER "puck-a"

// Pin D4 on the D1 mini is the same as LED_BUILTIN, but they also
// chose to use it as the default pin for this shield. If you want
// to print a translucent shell, you'll want to remap the control pin
// by cutting the D4 jumper and bridging the gap between another pad
// and the control line using solder. This keeps the built-in blue LED
// off.
#define LED_PIN D4

// If you're using the RGB LED shield, leave this set to seven.
#define LED_COUNT 7
#define LED_BRIGHTNESS 0x08

// If your WiFi network is congested, you may need to increase this.
#define HTTP_TIMEOUT_MS 2e3

// You can customize the HTTP port by changing this value.
#define HTTP_PORT 80
#define HTTP_SERVICE "http"
#define HTTP_PROTOCOL "tcp"

// Shouldn't need to edit any of these
#define RESPONSE_OK F("HTTP/1.1 200 OK")
#define RESPONSE_BAD_REQUEST F("HTTP/1.1 400 Bad Request")
#define RESPONSE_INTERNAL_SERVER_ERROR F("HTTP/1.1 500 Internal Server Error")
#define HEADER_ACCESS_CONTROL_ALLOW_ORIGIN F("Access-Control-Allow-Origin: *")
#define HEADER_ACCESS_CONTROL_ALLOW_HEADERS F("Access-Control-Allow-Headers: content-type")
#define HEADER_CONNECTION F("Connection: close")
#define HEADER_CONTENT_TYPE_JSON F("Content-type: application/json")

// Calculated using https://arduinojson.org/v6/assistant/
#define PACKET_SIZE JSON_OBJECT_SIZE(4) + 10

struct Settings {
  uint32_t color;
  uint8_t brightness;
};

struct RgbColor {
  uint8_t red;
  uint8_t green;
  uint8_t blue;

  RgbColor(uint32_t packed) {
    this->red = (uint8_t)(packed >> 16);
    this->green = (uint8_t)(packed >> 8);
    this->blue = (uint8_t)packed;
  }
};

Adafruit_NeoPixel* pixels;
WiFiServer* api;
Settings settings = Settings();

const uint32_t black = pixels->Color(0, 0, 0);

/**
 * If WiFi is not connected, allow up to 10 seconds for connection.
 */
bool checkWiFi() {
  if (WiFi.status() == WL_CONNECTED) {
    return true;
  }

  // give it ten seconds
  uint16_t timeoutMs = 10000;
  // half a second per tick
  const uint16_t delayMs = 500;

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_AP, WIFI_PSK);

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
 * Periodically attempt to reconnect to WiFi, blinking the LED
 * in between attempts.
 */
void blinkDeath() {
  bool blink = LOW;
  pinMode(LED_BUILTIN, OUTPUT);
  while (1) {
    digitalWrite(LED_BUILTIN, blink);
    blink = !blink;

    WiFi.reconnect();
    if (checkWiFi()) {
      digitalWrite(LED_BUILTIN, LOW);
      break;
    }
  }
}

/**
 * Set all pixels to the saved brightness/color values.
 */
void setPixels() {
  pixels->setBrightness(settings.brightness);

  // note: this code does not support > 256 LEDs
  for (uint8_t i = 0; i < LED_COUNT; i++) {
    pixels->setPixelColor(i, settings.color);
  }

  pixels->show();
}

/**
 * Set up hardware, restore settings from EEPROM, start web server.
 */
void setup() {
  // turn off built-in LED
  pinMode(LED_BUILTIN, HIGH);

  // set up WiFi radio
  WiFi.setSleepMode(WIFI_NONE_SLEEP);
  WiFi.hostname(DEVICE_IDENTIFIER);

  // set up MDNS
  MDNS.begin(DEVICE_IDENTIFIER);
  MDNS.addService(HTTP_SERVICE, HTTP_PROTOCOL, HTTP_PORT);

  // init LEDs
  pixels = new Adafruit_NeoPixel(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
  pixels->begin();
  pixels->setBrightness(LED_BRIGHTNESS);
  pixels->show();
  pixels->fill(black);
  pixels->show();

  // init HTTP
  api = new WiFiServer(HTTP_PORT);
  api->begin();

  // init settings from EEPROM
  EEPROM.begin(sizeof(Settings));

  if (EEPROM.percentUsed() >= 0) {
    EEPROM.get(0, settings);
    setPixels();
  } else {
    settings.color = black;
    settings.brightness = LED_BRIGHTNESS;

    EEPROM.put(0, settings);
    EEPROM.commit();
  }

  EEPROM.end();
}

/**
 * 1) Check for WiFi
 * 2) Check for HTTP connection
 * 3) Read/parse/respond to HTTP request
 * 4) If settings changed, update EEPROM
 *
 * Need to add authentication.
 */
void loop() {
  if (!checkWiFi()) {
    return blinkDeath();
  }

  MDNS.update();

  WiFiClient client = api->available();

  if (!client) {
    return;
  }

  uint32_t currentTime = millis();
  uint32_t lastTime = currentTime;

  String request;
  StaticJsonDocument<PACKET_SIZE> doc;

  while (!client.available() && (currentTime - lastTime) <= HTTP_TIMEOUT_MS) {
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
    return;
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

    return;
  }

  // skip all the headers
  // note: this is vulerable to DOS attacks
  while (!request.startsWith("{") && (currentTime - lastTime) <= HTTP_TIMEOUT_MS) {
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

    return;
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

  const uint32_t newColor = pixels->Color(doc["r"], doc["g"], doc["b"]);
  const uint8_t newBrightness = doc["br"];
  boolean settingsChanged = false;

  // sync settings if they have changed
  if (settings.color != newColor) {
    settings.color = newColor;
    settingsChanged = true;
  }

  if (settings.brightness != newBrightness) {
    settings.brightness = newBrightness;
    settingsChanged = true;
  }

  // update pixels and write new settings to EEPROM if necessary
  if (settingsChanged) {
    setPixels();

    EEPROM.begin(sizeof(Settings));
    EEPROM.put(0, settings);
    EEPROM.commit();
    EEPROM.end();
  }
}