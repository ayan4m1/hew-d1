#include <Arduino.h>
#include <SPI.h>
#include <config.h>

#ifndef CLASS_LOG
#define CLASS_LOG
class Log {
 private:
  static bool started;

 public:
  static void init();
  static void end();
  static void log(const char* format, ...);
};
#endif