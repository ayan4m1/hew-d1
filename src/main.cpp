#include <Adafruit_NeoPixel.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP_EEPROM.h>

#define WIFI_AP "bar"
#define WIFI_PSK "wi9NNYara"
#define DEVICE_IDENTIFIER "puck-a"

#define PACKET_SIZE JSON_OBJECT_SIZE(4) + 10

#define LED_PIN D4
#define LED_COUNT 7
#define LED_BRIGHTNESS 0x08

#define HTTP_TIMEOUT_MS 2e3
#define HTTP_PORT 80
#define HTTP_SERVICE "http"
#define HTTP_PROTOCOL "tcp"

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

void setPixels() {
  pixels->setBrightness(settings.brightness);

  for (uint8_t i = 0; i < LED_COUNT; i++) {
    pixels->setPixelColor(i, settings.color);
  }

  pixels->show();
}

void setup() {
  // turn off built-in LED
  pinMode(LED_BUILTIN, LOW);

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

  while (client.connected() && (currentTime - lastTime) <= HTTP_TIMEOUT_MS) {
    currentTime = millis();

    String request;
    StaticJsonDocument<PACKET_SIZE> doc;

    if (client.available()) {
      request = client.readStringUntil('\n');

      // ensure this is a POST request
      if (!request.startsWith("POST")) {
        client.println(F("HTTP/1.1 400 Bad Request"));
        client.println(F("Connection: close"));
        client.println();
        client.println();
        break;
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
        client.println(F("HTTP/1.1 500 Internal Server Error"));
        client.println(F("Connection: close"));
        client.println();
        client.println(error.c_str());
        client.println();
      } else {
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

        client.println(F("HTTP/1.1 200 OK"));
        client.println(F("Content-type: application/json"));
        client.println(F("Connection: close"));
        client.println();
        client.println();
      }
    }
  }

  if (client.connected()) {
    client.stop();
  }
}