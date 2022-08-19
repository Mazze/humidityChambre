#ifndef GLOBALS_H
#define GLOBALS_H
#include "configObjectStruct.h"

typedef struct stateStruct_Tag{

  bool vaporizerOn = false;
  bool DehumidifyOn = false;
  float temperature = -100;
  float humidity =-1;
  bool isAzureConnected =false;
  bool needReconnectAzure = false;
  int ID ;
} stateStruct;
extern stateStruct currentState;
extern configObjectStruct g_configObject ;
#endif