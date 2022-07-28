#ifndef CONFIGOBJECTSTRUCT_H
#define CONFIGOBJECTSTRUCT_H

#include <LittleFS.h>


#define FIRMWAREVERSION_MAJOR 1
#define FIRMWAREVERSION_MINOR 3
#define FIRMWAREVERSION_BUILD 7

struct configObjectStruct{

  char ssid[64];
  char PW[64];
  char SCOPE_ID[20];
  char DEVICE_ID[20];
  char DEVICE_KEY[50];  
  char firmwareUrlAddon[256];
  char firmwareUpdateState[20];
  char ntpserver1[50];
  char ntpserver2[50];
};

bool setStoredConfig(configObjectStruct* co );

bool getStoredConfig(configObjectStruct* co );
void getStoredConfigJson(String* configJson);

#endif