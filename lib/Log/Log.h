#include <Arduino.h>
#include <SPI.h>

#ifndef CLASS_LOG
#define CLASS_LOG
class Log {
 private:
  static bool started;

 public:
  static void init(uint32_t);
  static void end();
  static void log(const char* format, ...);
};
#endif