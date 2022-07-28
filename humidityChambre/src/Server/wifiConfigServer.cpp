#include <FS.h>
#include <LittleFS.h>
#include <ESPmDNS.h>
#include "wifiConfigServer.h"
#include "../configObjectStruct.h"
#include <stdlib.h>

#include <uri/UriBraces.h>
#include <uri/UriRegex.h>

//const char* serverHeader = "<h2>Temperature Prob - Wifi</h2>";
//const char* serverWifiForm = "<form enctype=\"multipart/form-data\" action=\"/saveWifi\" method=\"POST\"><label for=\"ssid\">Wifi Name(SSID):</label><br><input type=\"text\" id=\"ssid\" name=\"ssid\" ><br><label for=\"pw\">Password:</label><br><input type=\"text\" id=\"pw\" name=\"pw\" ><br><label for=\"ssid\">SCOPE_ID:</label><br><input type=\"text\" id=\"SCOPE_ID\" name=\"SCOPE_ID\" ><label for=\"DEVICE_ID\">DEVICE_ID:</label><br><input type=\"text\" id=\"DEVICE_ID\" name=\"DEVICE_ID\" ><br>label for=\"DEVICE_KEY\">DEVICE_KEY:</label><br><input type=\"text\" id=\"DEVICE_KEY\" name=\"DEVICE_KEY\" ><br><br><br><input type=\"submit\" value=\"Save and Restart\"></form>";
static const char TEXT_PLAIN[]   = "text/plain";
 

String fileReadline(File* file)
{
    String ret;
    ret.reserve(64);
    char lastChar =0;
    //Serial.println("3.1");  
    while ((file->available() ) &&(lastChar!= 10) ) {
        lastChar = file->read();
        //Serial.println(lastChar);  
        ret += String(lastChar);
    }
    return ret ;
};

String fileReadAll(File* file)
{
    String ret;
    ret.reserve(64);
    char lastChar =0;
    //Serial.println("3.1");  
    while (file->available() )   {
        lastChar = file->read();
        //Serial.println(lastChar);  
        ret += String(lastChar);
    }
    return ret ;
};


String getContentType(String path) {
  String dataType = "text/plain";
  if (path.endsWith("/")) {
    path += "index.htm";
  }

  if (path.endsWith(".src")) {
    path = path.substring(0, path.lastIndexOf("."));
  } else if (path.endsWith(".htm")) {
    dataType = "text/html";
  } else if (path.endsWith(".html")) {
    dataType = "text/html";
  } else if (path.endsWith(".css")) {
    dataType = "text/css";
  } else if (path.endsWith(".js")) {
    dataType = "application/javascript";
  } else if (path.endsWith(".png")) {
    dataType = "image/png";
  } else if (path.endsWith(".gif")) {
    dataType = "image/gif";
  } else if (path.endsWith(".jpg")) {
    dataType = "image/jpeg";
  } else if (path.endsWith(".ico")) {
    dataType = "image/x-icon";
  } else if (path.endsWith(".xml")) {
    dataType = "text/xml";
  } else if (path.endsWith(".pdf")) {
    dataType = "application/pdf";
  } else if (path.endsWith(".zip")) {
    dataType = "application/zip";
  }
  return dataType;
}

void wifiConfigServer::handlePlain() {
    if (this->server->method() != HTTP_POST) {
        this->server->send(405, "text/plain", "Method Not Allowed");    
    } else {    
        Serial.println("Post Data:");        
        // try 
        // { 
        Serial.println(this->server->arg("plain"));
        // }
        // catch(const std::exception& e){}
        Serial.println("ssid Data:");        
        // try 
        // { 
            Serial.println(this->server->arg("ssid"));
        // }
        // catch(const std::exception& e){}
        this->server->send(200, "text/plain", "POST body was:\n" + this->server->arg("plain"));    
    }
};


bool wifiConfigServer::handleFileRead(String assetFile) {    
    String path = "/assets/"+assetFile;
    if (this->server->method() != HTTP_GET) {
        this->server->send(405, "text/plain", "Method Not Allowed");    
    } else {    
        Serial.println("Asset request:");    
        Serial.println(path)    ;
        Serial.println(this->server->arg("plain"));
        
        if (path != "/" && !LittleFS.exists(path)) {
            Serial.println("BAD PATH");  
            replyBadRequest("BAD PATH");
        }
        if (!LittleFS.exists(path)) {
            Serial.println("BAD FILE");  
            replyBadRequest("BAD FILE");
        }
        else
        {        
            // Replace with  document.querySelector("#ipText").innerHTML = myJson.ip;
            //https://www.kirupa.com/html5/making_http_requests_js.htm
            
            //What type of asses
            String contentType = getContentType(path);
            Serial.println("  FILE " +contentType);  
            File file = LittleFS.open(path, "r");
            if (this->server->streamFile(file, contentType) != file.size()) {
            //DBG_OUTPUT_PORT.println("Sent less data than expected!");
            }
            file.close();    
            return true;
        }
        return false;
        //this->server->send(200, "text/plain", "POST body was:\n" + this->server->arg("plain"));    
    }
}

void wifiConfigServer::handleNotFound() {    
    if (this->handleFileRead(this->server->uri())) {
        return;
    }
    this->server->send(404, FPSTR(TEXT_PLAIN), "Request not found");
}

void wifiConfigServer::replyBadRequest(String msg) {
    //DBG_OUTPUT_PORT.println(msg);
    this->server->send(400, FPSTR(TEXT_PLAIN), msg + "\r\n");
}

void wifiConfigServer::handlePostConfig(){
  String configPart = this->server->pathArg(0);
  Serial.println("Config : " + configPart);
  if (strncmp("saveWifi",this->server->pathArg(0).c_str(),8 ) ==0)
  {
    
      //Save the wifi config
      if (this->server->hasArg("ssid")&&this->server->hasArg("pw"))
      {
        Serial.println("save wifi");
        configObjectStruct co;
        getStoredConfig(&co );
        strcpy((co.ssid), this->server->arg("ssid").c_str());
        strcpy((co.PW), this->server->arg("pw").c_str());


        setStoredConfig(&co );
        this->server->sendHeader("Location","/");
        this->server->send(303, FPSTR(TEXT_PLAIN), "action:1");
      }   
      else
      {
        this->server->send(400, FPSTR(TEXT_PLAIN), "Required data not send");
      }

  }


  String post  = this->server->arg("plain");
  Serial.println(post);
  Serial.print("number of ars : ");
  Serial.println(this->server->args()  );
  for (int i = 0 ;i< this->server->args();i++)
    Serial.println(this->server->arg(i));
  Serial.println("done");
  //  this->server->send(200, FPSTR(TEXT_PLAIN), "Saved");
}

void wifiConfigServer::handleGetConfig(){
  
  String finalJson ;
  getStoredConfigJson(&finalJson);
  this->server->send(200, "application/json", finalJson);    
}

void wifiConfigServer::handleGetDeviceInfo(){
  String mac= "\"mac\":\""+ WiFi.macAddress()+"\"" ;
  String finalJson = "{" + mac +"}";
  this->server->send(200, "application/json", finalJson);    
}

void wifiConfigServer::handlePostAction(){
  if (this->server->uri()=="restart" )
  {
    Serial.println("Restart");
    Serial.flush();
    delay(200);
    this->server->send(200, "text/plain", "<!DOCTYPE html><html><body><h2>Restarting </h2></body></html> ");    
    ESP.restart();
  }
  else
  {
      this->server->send(418, "text/plain", "Action Not Allowed");
  }
}



void wifiConfigServer::setupServer(int port)
{
    
    Serial.println("start setup");
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);
    Serial.print("MAC address: ");
    Serial.println(WiFi.macAddress()); 
    
    if (!MDNS.begin("humidity")) {
        Serial.println("Error setting up MDNS responder!");        
    }
    Serial.println("MDNS done");
    this->server = new WebServer(port);
    this->server->on("/", HTTP_GET, [this]() {
        this->handleFileRead("/index.html");        
        });        
    
    this->server->on(UriBraces("/config/{}"),  [this]() {
                Serial.println("onconfig");
                if (this->server->method() == HTTP_POST) {
                  this->handlePostConfig();
                } 
                else if (this->server->method() == HTTP_GET){
                  this->handleGetConfig();
                }
                else { 
                  this->server->send(405, "text/plain", "Method Not Allowed");                            
                }                                
              }
              );
    this->server->on("/deviceinfo",  [this]() {
                Serial.println("ondeviceinfo");
                if (this->server->method() != HTTP_GET) {
                  this->server->send(405, "text/plain", "Method Not Allowed");                            
                } 
                else{
                  this->handleGetDeviceInfo();
                }                
              });
    this->server->on("/action",  [this]() {
                Serial.println("onaction");
                if (this->server->method() != HTTP_POST) {
                  this->server->send(405, "text/plain", "Method Not Allowed");                            
                } 
                else{
                  this->handlePostAction();
                }                
              });
    Serial.println("Handels done");
    // this->server->on("/saveWifi",  [this]() {
    //             Serial.println("onsaveWifi");
    //             if (this->server->method() != HTTP_POST) {
    //                     this->server->send(405, "text/plain", "Method Not Allowed");    
    //                 } else { 
    //                     Serial.println("pw Data:");                    
    //                     Serial.println(this->server->arg("pw"));
    //                     Serial.println("ssid Data:");                            
    //                     Serial.println(this->server->arg("ssid"));                                                
    //                     configObjectStruct co;
    //                     getStoredConfig(&co );
    //                     strcpy((co.ssid), this->server->arg("ssid").c_str());
    //                     strcpy((co.PW), this->server->arg("pw").c_str());

                        
    //                     setStoredConfig(&co );

    //                 }
                                
    //             }
    //             );
    // this->server->on("/restart",  [this]() {
    //             Serial.println("onsaveWifi");
    //             if (this->server->method() != HTTP_POST) {
    //                     this->server->send(405, "text/plain", "Method Not Allowed");    
    //                 } else { 
            
    //                     Serial.println("Restart");
    //                     Serial.flush();
    //                     delay(200);
    //                     ESP.restart();
    //                     this->server->send(405, "text/plain", "<!DOCTYPE html><html><body><h2>Restarting </h2></body></html> ");    
    //                 }
                                
    //             }
    //             );
    //         this->server->on("/saveCloud",  [this]() {
    //             Serial.println("onsaveWifi");
    //             if (this->server->method() != HTTP_POST) {
    //                     this->server->send(405, "text/plain", "Method Not Allowed");    
    //                 } else { 

                        
    //                     configObjectStruct co;
    //                     getStoredConfig(&co );
    //                     strcpy((co.SCOPE_ID), this->server->arg("SCOPE_ID").c_str());
    //                     strcpy((co.DEVICE_ID), this->server->arg("DEVICE_ID").c_str());
    //                     strcpy((co.DEVICE_KEY), this->server->arg("DEVICE_KEY").c_str());

    //                     setStoredConfig(&co );

    //                 }
                                
    //             }
    //             );      
    // this->server->on("/saveNTP",  [this]() {
    //             Serial.println("onsaveNTP");
    //             if (this->server->method() != HTTP_POST) {
    //                     this->server->send(405, "text/plain", "Method Not Allowed");    
    //                 } else { 
    //                     Serial.println("NTP1:");                    
    //                     Serial.println(this->server->arg("NTP1"));
    //                     Serial.println("NTP2:");        
    //                     Serial.println(this->server->arg("NTP2"));
    //                     configObjectStruct co;
    //                     getStoredConfig(&co );
    //                     strcpy((co.ntpserver1), this->server->arg("NTP1").c_str());
    //                     strcpy((co.ntpserver2), this->server->arg("NTP2").c_str());                            
    //                     setStoredConfig(&co );
    //                 }
                                
    //             }
    //             );    
    this->server->onNotFound( [this]() {this->handleNotFound();} );
    
    this->server->begin();
    Serial.println("Up");
    MDNS.addService("http", "tcp", 80);
    Serial.println("MDNS service added");
}




void wifiConfigServer::loopItem(unsigned long lastTick)
{
    //Serial.printf("loopItem 1\n");

    //const char root = '/';
    //listDir(LittleFS, "/", 2);    
    unsigned long ms = millis();    
    this->server->handleClient();    
    //Serial.printf("%d\n", lastTick);
    //Serial.printf("%d\n", ms);
    if (ms - lastTick > 15*60*1000) 
    {
        Serial.printf("loopItem 4\n");
           // ESP.restart();
    }
}
