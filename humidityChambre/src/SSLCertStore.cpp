
#include "FS.h"
#include <LittleFS.h>
#include "SSLCertStore.h"



SSLCertStore::SSLCertStore()
{}
bool SSLCertStore::loadCertsFromFile(String Filename)
{
  File file = LittleFS.open(Filename, "r");
  if (!file.available())
  {
      Serial.printf  ("File not available !!");
      return false;
  }
  
  this->myCerts.clear();
  myCerts.reserve(64);
  while (file.available() )   {      
      myCerts += String(  (char) file.read() );
  }
  
  return true;
}

const char* SSLCertStore::getCerts()
{ return  myCerts.c_str();
 }