#ifndef CONFIGOBJECTSTRUCT_H
#define CONFIGOBJECTSTRUCT_H

#include <LittleFS.h>


#define FIRMWAREVERSION_MAJOR 1
#define FIRMWAREVERSION_MINOR 3
#define FIRMWAREVERSION_BUILD 7

extern const char* pFirmwareUpdateState;
extern const char* pFirmwareVersion ;
extern const char* pDoorOpenState;
extern const char* pVaporiserOnState;
extern const char* pDeHumidifierOnState;
extern const char* sFirmwareURL ;
extern const char* sSsid ;
extern const char* sSsidPW ;
extern const char* sNtpserver1 ;
extern const char* sNtpserver2 ;
extern const char* sTargetHumid ;
extern const char* sTargetHumidTolerance ;
extern const char* sDehumidityPin ;
extern const char* sVaporizerPin ;
extern const char* sTypeSensor ;
extern const char* sPinSDA ;
extern const char* sPinSCL ;
extern const char* sPinFan ;
extern const char* sFanSpeed ;
extern const char* tHumidity ;
extern const char* tTemperature ;

extern const int numberOfProperties;
extern const char* propertyTags[];
extern const int numberOfTag ;
extern const char* settingTags[];


typedef struct configObjectStructTag{
  char ssid[64];
  char PW[64];
  char SCOPE_ID[20];
  char DEVICE_ID[20];
  char DEVICE_KEY[50];  
  char firmwareUrlAddon[256];
  char firmwareUpdateState[20];
  char ntpserver1[50];
  char ntpserver2[50];
  int targetHumid;
  int targetHumidTolerance;
  uint8_t pinDehumidity;
  uint8_t pinVaporizer;
  uint8_t typeSensor;
  uint8_t pinSDA;
  uint8_t pinSCL;
  uint8_t pinFan;
  uint8_t pinDoorswitch;
  uint8_t pinControllSwitch1;
  uint8_t pinControllSwitch2;
  uint8_t speedFan;

}configObjectStruct;

void printConfig(configObjectStruct* co );
bool setStoredConfig(configObjectStruct* co );

bool getStoredConfig(configObjectStruct* co );

bool getConfigFromSettingAsString(const char* settingName, char* buffer, int length);
void setConfigFromSettingAsString(const char* settingName, char* buffer,configObjectStruct* cop=NULL );

bool getStateAsString(const char* stateElement, char* buffer, int length);
#endif