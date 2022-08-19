#include "logging.h"

using namespace logging;

void logging::reportOutput(String msg, reportLevel level )
{
//   //level 
//   //  0 = information
//   //  1 = warning 
//   //  2 = error
switch (level)
{
    case 0:LOG_VERBOSE(msg.c_str());break;
    case 1:LOG_VERBOSE(msg.c_str());break;
    case 2: LOG_ERROR(msg.c_str());
}

//   //listOfErrors.push_front(F("SSD1306 allocation failed"))
}