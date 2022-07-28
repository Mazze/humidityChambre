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
#endif