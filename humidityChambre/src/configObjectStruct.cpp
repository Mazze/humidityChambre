#include"configObjectStruct.h"
#include "logging.h"
#include <FS.h>
#include <LittleFS.h>


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
  return true;
}

void getStoredConfigJson(String* configJson)
{
  configObjectStruct co;
  int i = sizeof(configObjectStruct::ssid );
  if (getStoredConfig(&co ))
  {
    *configJson = "{";// +
                  // "\"ssid\""+co.ssid +
                  // "}");
  }
  else
    *configJson=  "{\"error\":\"Faild to read config\"}";
} 