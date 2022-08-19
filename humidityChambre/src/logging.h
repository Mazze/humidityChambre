#ifndef LOGGING_H
#define LOGGING_H

#include <WiFi.h> // This is the incorrect way to import String 


void test (String s);

namespace logging
{

enum reportLevel
{ 
  information = 0 ,
  warning = 1, 
  error = 2,
};

void reportOutput(String msg, logging::reportLevel level = information);

}

#ifndef LOG_VERBOSE
// #ifdef ARDUINO
//#define SERIAL_PRINT Serial.print
// #else
 #define SERIAL_PRINT printf
// #endif

#define SERIAL_VERBOSE_LOGGING_ENABLED 1

#ifndef LOG_VERBOSE
#if SERIAL_VERBOSE_LOGGING_ENABLED != 1
#define LOG_VERBOSE(...)
#else
#define LOG_VERBOSE(...)       \
  do {                         \
    SERIAL_PRINT("  - ");      \
    SERIAL_PRINT(__VA_ARGS__); \
    SERIAL_PRINT("\r\n");      \
  } while (0)
#endif  // SERIAL_VERBOSE_LOGGING_ENABLED != 1

// Log Errors no matter what
#define LOG_ERROR(...)                                            \
  do {                                                            \
    SERIAL_PRINT("X - Error at %s:%d\r\n\t", __FILE__, __LINE__); \
    SERIAL_PRINT(__VA_ARGS__);                                    \
    SERIAL_PRINT("\r\n");                                         \
  } while (0)
#endif  // !LOG_VERBOSE
#endif  // LOG_VERBOSE
#endif