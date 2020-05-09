#include "Log.h"

bool Log::started = false;

void Log::init(uint32_t baudRate) {
  Serial.begin(baudRate);

  while (!Serial) {
    delayMicroseconds(100);
  }

  Log::log("Initialized serial");
  Log::started = true;
};

void Log::end() {
  Serial.end();

  Log::started = false;
};

void Log::log(const char* format, ...) {
  if (!Log::started) {
    return;
  }

  // interpolate arguments into format string
  va_list args;
  va_start(args, format);
  char buffer[256];
  vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  // print data to Serial
  Serial.println(buffer);
}