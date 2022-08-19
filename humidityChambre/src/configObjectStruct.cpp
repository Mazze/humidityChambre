#include"configObjectStruct.h"
#include "logging.h"
#include <FS.h>
#include <LittleFS.h>
#include "globals.h"

#define FIRMWAREVERSION_MAJOR 0
#define FIRMWAREVERSION_MINOR 1
#define FIRMWAREVERSION_BUILD __DATE__,__TIME__
#define PINDOORSWITCH 5
#define PINCONTROLLSWITCH1 6
#define PINCONTROLLSWITCH2 7
//Reported Properties
const char* pFirmwareUpdateState =  "firmwareUpdateState";
const char* pFirmwareVersion =  "FirmwareVersion";
const char* pDoorOpenState ="pDoorOpen";
const char* pVaporiserOnState ="pVaporOn";
const char* pDeHumidifierOnState ="pDeHumifOn";
//Synct Settings
const char* sFirmwareURL =   "FirmwareURL";
const char* sSsid = "ssid";
const char* sSsidPW =  "cssidPW";
const char* sNtpserver1 =  "ntpserver1";
const char* sNtpserver2 =  "ntpserver2";
const char* sTargetHumid = "ctargetHumid";
const char* sTargetHumidTolerance = "ctolTargetHumid";
const char* sDehumidityPin = "cDehumidPin";
const char* sVaporizerPin = "cVapoPin" ;
const char* sTypeSensor = "ctypeSensor";
const char* sPinSDA =  "cpinSDA";
const char* sPinSCL =  "cpinSCL";
const char* sPinFan = "sPinFan";
const char* sFanSpeed = "sFanSpeed";
const int numberOfTag =13 ;
const int numberOfProperties =5 ;
//Telemetry Data
const char* tHumidity =  "tHumid";
const char* tTemperature =  "tTemp";



const char* settingTags [numberOfTag]= {sFirmwareURL,
                      sSsid, sSsidPW, sNtpserver1, sNtpserver2 ,
                      sTargetHumid,sTargetHumidTolerance, 
                      sDehumidityPin, sVaporizerPin, sPinSDA,sPinSCL,
                      sPinFan, sFanSpeed  }; 
const char* propertyTags []= {pFirmwareVersion ,
                      pFirmwareUpdateState,
                      pDoorOpenState,
                      pVaporiserOnState,
                      pDeHumidifierOnState}; 


inline void initCOStruct(configObjectStruct* co)
{
  memset(co,sizeof(configObjectStruct),0);
  
  co->targetHumid=-1;
  co->targetHumidTolerance=-1;
  co->pinDehumidity=1;
  co->pinVaporizer=2;
  co->typeSensor=3;
  co->pinSDA=18;
  co->pinSCL=19;
  co->pinFan=3;
  co->pinDoorswitch=PINDOORSWITCH;
  co->pinControllSwitch1=PINCONTROLLSWITCH1;
  co->pinControllSwitch2=PINCONTROLLSWITCH2;
  co->speedFan=128;
} 


void printConfig(configObjectStruct* co )
{
  char text[300];
  Serial.println("*********************************");
  Serial.println("Config object ");
  sprintf(text,"SSID : %s  ;PW %s   ",co->ssid ,co->PW );
  Serial.println(text);
  sprintf(text,"SCOPE_ID : %s  ;DEVICE_ID %s   ",co->SCOPE_ID ,co->DEVICE_ID );
  Serial.println(text);
  sprintf(text,"DEVICE_KEY : %s     ",co->DEVICE_KEY );
  Serial.println(text);
  sprintf(text,"firmwareUrlAddon : %s     ",co->firmwareUrlAddon );
  Serial.println(text);
  sprintf(text,"ntpserver1 : %s  ;ntpserver2 %s   ",co->ntpserver1 ,co->ntpserver2 );
  Serial.println(text);
  // char firmwareUpdateState[20];
  sprintf(text,"targetHumid : %d  ;targetHumidTolerance %d   ",co->targetHumid ,co->targetHumidTolerance );
  Serial.println(text);
  sprintf(text,"Pins (Hum,Vap,SDA,SC:): %d ; %d ; %d ;%d   ",co->pinDehumidity ,co->pinVaporizer, co->pinSDA, co->pinSCL );
  Serial.println(text);  
  sprintf(text,"Switch Pins (Door, SW1, SW2 ): %d ; %d ; %d   ",co->pinDoorswitch ,co->pinControllSwitch1, co->pinControllSwitch2 );
  Serial.println(text);
  sprintf(text,"Fan config (Speed, pin): %d ; %d ",co->speedFan ,co->pinFan );
  Serial.println(text);

  Serial.println("*********************************");
}

bool setStoredConfig(configObjectStruct* co )
{  
  if(!LittleFS.begin(false)){       
  }
  logging::reportOutput("Save config");  
  //Read the file
  File f = LittleFS.open("/baseconfig.bin", "w");
  if (!f)
  {
    //No config
    logging::reportOutput("could not open file for write");
    return false;
  }  
  f.write(reinterpret_cast<uint8_t*>(co), sizeof(configObjectStruct));
  f.close();
  return true;
  };

bool getStoredConfig(configObjectStruct* co )
{
  initCOStruct(co);
  if(!LittleFS.begin(false)){
      Serial.println("LittleFS Mount Failed");
      return false;
  }
  //Read the file
  File f = LittleFS.open("/baseconfig.bin", "r");
  if (!f)
  {
    //No config
    Serial.println("could not open file");
    return false;
  }
  if (f.size()!=sizeof(configObjectStruct))
  {
    Serial.println(F("file size mismatch"));
    return false;
  }
  f.read(reinterpret_cast<uint8_t*>(co), sizeof(configObjectStruct) ); 
  f.close();

  //Set defaults, I should not do it, but I do not want to put it in the 
  // Cloud, no need.
  co->pinDoorswitch=PINDOORSWITCH;
  co->pinControllSwitch1=PINCONTROLLSWITCH1;
  co->pinControllSwitch2=PINCONTROLLSWITCH2;
  return true;
}

void setStrFromBuffer(void* target, uint targetSize, void* buffer)
{
  //LOG_VERBOSE("New Firmware URL start savein"); 
  memset(target,0,targetSize);
  //LOG_VERBOSE("....."); 
  memcpy(target,buffer,strlen((char*)buffer));  
}

void setConfigFromSettingAsString(const char* settingName, char* buffer,configObjectStruct* cop )
{
  LOG_VERBOSE("Save : %s",settingName ); 
  configObjectStruct* co;
  if (cop !=NULL)
  {      
    co=cop;
  }
  else 
  {    
    co = (configObjectStruct*) malloc(sizeof(configObjectStruct));
    getStoredConfig(co );  
  }  
  
  if(strcmp(settingName, sFirmwareURL)==0)
  {
    //LOG_VERBOSE("New Firmware URL start savein"); 
    memset(co->firmwareUrlAddon,0,sizeof(co->firmwareUrlAddon));
    //LOG_VERBOSE("....."); 
    memcpy(co->firmwareUrlAddon,buffer,strlen(buffer));
    LOG_VERBOSE("New Firmware URL: ?%s?", co->firmwareUrlAddon);      
  }
  else if(strcmp(settingName, sSsid)==0)
  {
    setStrFromBuffer(co->ssid,sizeof(co->ssid),buffer);    
    LOG_VERBOSE("New ssid: %s", co->ssid);      
  }
  else if(strcmp(settingName, sNtpserver1)==0)
  {
    setStrFromBuffer(co->ntpserver1,sizeof(co->ntpserver1),buffer);
    LOG_VERBOSE("New ntpserver1: %s", co->ntpserver1);      
  }
  else if(strcmp(settingName, sNtpserver2)==0)
  {
    setStrFromBuffer(co->ntpserver2,sizeof(co->ntpserver2),buffer);
    LOG_VERBOSE("New ntpserver2: %s", co->ntpserver2);      
  }
  else if(strcmp(settingName, sSsidPW)==0)
  {
    setStrFromBuffer(co->PW,sizeof(co->PW),buffer);    
    LOG_VERBOSE("New PW: %s", co->PW);          
  }
  else if(strcmp(settingName, sDehumidityPin)==0)
  {      
      sscanf(buffer, "%d",
                     &co->pinDehumidity);      
      LOG_VERBOSE("New pinDehumidity: %d", co->pinDehumidity);      
  }
  else if(strcmp(settingName, sVaporizerPin)==0)
  {      
      sscanf(buffer, "%d",
                     &co->pinVaporizer);      
      LOG_VERBOSE("New pinVaporizer: %d", co->pinVaporizer);      
  }
  else if(strcmp(settingName, sPinSDA)==0)
  {      
      sscanf(buffer, "%d",
                     &co->pinSDA);      
      LOG_VERBOSE("New sPinSDA: %d", co->pinSDA);      
  }
  else if(strcmp(settingName, sPinSCL)==0)
  {      
      sscanf(buffer, "%d",
                     &co->pinSCL);      
      LOG_VERBOSE("New sPinSDA: %d", co->pinSCL);      
  }
  else if(strcmp(settingName, sPinFan)==0)
  {      
      sscanf(buffer, "%d",
                     &co->pinFan);      
      LOG_VERBOSE("New sPinFAN: %d", co->pinFan);      
  }
  else if(strcmp(settingName, sFanSpeed)==0)
  {      
      sscanf(buffer, "%d",
                     &co->speedFan);      
      LOG_VERBOSE("New sFanSpeed: %d", co->speedFan);      
  }
  else if(strcmp(settingName, sTargetHumid)==0)
  {      
      sscanf(buffer, "%d",
                     &co->targetHumid);      
      LOG_VERBOSE("New targetHumid: %d", co->targetHumid);      
  }
  else if(strcmp(settingName, sTargetHumidTolerance)==0)
  {
      //LOG_VERBOSE("New Refrashrate as string: %s", buffer);       
      sscanf(buffer, "%d",
                     &co->targetHumidTolerance);      
      LOG_VERBOSE("New targetHumidTolerance: %d", co->targetHumidTolerance);      
  }

  else {
    LOG_ERROR("Received unknown settin: %s",settingName);
    if (cop ==NULL)
    {
      free (co);
    }
    return;
  }  
  setStoredConfig( co );
  if (cop ==NULL)
  {
      free (co);
  }
}

// void getStoredConfigJson(String* configJson)
// {
//   configObjectStruct co;  
//   if (getStoredConfig(&co ))
//   {
//     *configJson = "{";// +
//                   // "\"ssid\""+co.ssid +
//                   // "}");
//   }
//   else
//     *configJson=  "{\"error\":\"Faild to read config\"}";
// } 

bool getConfigFromSettingAsString(const char* settingName, char* buffer, int length)
{  
  configObjectStruct co;
  getStoredConfig(&co ); 
  // {
  
  // sDehumidityPin, sVaporizerPin, sPinSDA,sPinSCL,
  // sPinFan, sFanSpeed  }; 
  if (strcmp(settingName, pFirmwareVersion)==0)
  {
    //LOG_VERBOSE("Firmware version" );
    snprintf(buffer, length, "\"%d.%02d.%02d\"",
                     FIRMWAREVERSION_MAJOR,FIRMWAREVERSION_MINOR,FIRMWAREVERSION_BUILD );    
  }
  else if(strcmp(settingName, sFirmwareURL)==0)
  {    
    snprintf(buffer,length,"\"%s\"",co.firmwareUrlAddon);
    // memcpy(buffer,&co.firmwareUrlAddon[1],strlen(co.firmwareUrlAddon)-2);           
  }
  else if(strcmp(settingName, pFirmwareUpdateState)==0)
  {    
    memcpy(buffer,"\"Not Set\"",9);          
  }
  else if(strcmp(settingName, sTargetHumid)==0)
    snprintf(buffer, length, "%d",co.targetHumid);    
  else if(strcmp(settingName, sTargetHumidTolerance)==0)
    snprintf(buffer, length, "%d",co.targetHumidTolerance);    
  else if(strcmp(settingName, sSsid)==0)
    snprintf(buffer, length, "\"%s\"",co.ssid);    
  else if(strcmp(settingName, sSsidPW)==0)
    snprintf(buffer, length, "\"%s\"",co.PW);    
  else if(strcmp(settingName, sNtpserver1)==0)
    snprintf(buffer, length, "\"%s\"",co.ntpserver1);    
  else if(strcmp(settingName, sNtpserver2)==0)
    snprintf(buffer, length, "\"%s\"",co.ntpserver2);    
  else if(strcmp(settingName, sDehumidityPin)==0)
    snprintf(buffer, length, "%d",co.pinDehumidity);    
  else if(strcmp(settingName, sVaporizerPin)==0)
    snprintf(buffer, length, "%d",co.pinVaporizer);    
  else if(strcmp(settingName, sPinSDA)==0)
    snprintf(buffer, length, "%d",co.pinSDA);    
    else if(strcmp(settingName, sPinSCL)==0)
    snprintf(buffer, length, "%d",co.pinSCL);    
    else if(strcmp(settingName, sPinFan)==0)
    snprintf(buffer, length, "%d",co.pinFan);    
    else if(strcmp(settingName, sFanSpeed)==0)
    snprintf(buffer, length, "%d",co.speedFan);    

  else {
    LOG_VERBOSE("Unknown" );
    snprintf(buffer, length, "  ");
    return false ;
  } 
  return true ;
}



bool getStateAsString(const char* stateElement, char* buffer, int length)
{
  if (strcmp(stateElement, pFirmwareVersion)==0)
  {
    //LOG_VERBOSE("Firmware version" );
    snprintf(buffer, length, "\"%d.%02d.%02d\"",
                     FIRMWAREVERSION_MAJOR,FIRMWAREVERSION_MINOR,FIRMWAREVERSION_BUILD );    
  }
 
  else if(strcmp(stateElement, pFirmwareUpdateState)==0)
  {    
    memcpy(buffer,"\"Not Set\"",9);          
    buffer[10]=0;
  }
  else if(strcmp(stateElement, pDoorOpenState)==0)
    snprintf(buffer, length, "%d",(int) currentState.isDoorOpen);    
  else if(strcmp(stateElement, pVaporiserOnState)==0)
    snprintf(buffer, length, "%d",(int) currentState.vaporizerOn);    
  else if(strcmp(stateElement, pDeHumidifierOnState)==0)
    snprintf(buffer, length, "%d",(int) currentState.DehumidifyOn);    
  else {
    //LOG_VERBOSE("Unknown" );
    snprintf(buffer, length, "  ");
    return false ;
  } 
  return true ;
}