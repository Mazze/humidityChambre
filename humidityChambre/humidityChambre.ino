

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>
#include <WiFi.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <FS.h>
#include <LittleFS.h>

#include "src/configObjectStruct.h"
#include "src/Server/wifiConfigServer.h"
#include "src/logging.h"

// I2C Defintion 
#define SDApin 18
#define SCLpin 19
TwoWire I2C_0 = TwoWire(0);

//Display Defintion
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C 
#define SSD1306_NO_SPLASH
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2C_0, OLED_RESET);


#define PIN 8
#define NUMPIXELS 1
Adafruit_NeoPixel pixels(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

#define WIFI_AP_SSID "Humidity"
#define WIFI_AP_PASSWORD "Humidity"
#define MAXWIFICONNECTIONTRY 5

using namespace logging;

bool g_isUpdatein = false;
unsigned long g_lastTick = 0;
wifiConfigServer* g_configServer=NULL;
//list<String> listOfErrors()

char g_charBuffer[50];

void displayInformation(const char *msg, const char *msg2 = NULL,const char *msg3 =NULL ) 
{
  char* l = strcpy(g_charBuffer,msg);  
  if ( msg2 != NULL)
  {
    l= g_charBuffer+ strlen(g_charBuffer);
    l = strcpy(l,msg2);
    if (msg3!=NULL)
      strcpy(l,msg3);
  }
  display.clearDisplay();
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(g_charBuffer);
  display.display();
  
  delay(500);
}

int connect_wifi(const char* wifi_ssid, const char* wifi_password,int maxTry) {
  WiFi.begin(wifi_ssid, wifi_password);
  reportOutput("Connecting to WiFi..");
  int countTry=0;
  while (WiFi.status() != WL_CONNECTED) {
    countTry ++;
    reportOutput("Failed try of wifi connection");
    delay(500);
    if (maxTry< countTry)
      break;
  }
  return countTry;
}

void setClock() {
  configObjectStruct co ;
  bool configLoaded = getStoredConfig(&co );
  configTime(0, 0, co.ntpserver1, co.ntpserver2);  // UTC  
  time_t now = time(nullptr);
  int i =0;
  while (now < 8 * 3600 * 2) {
    yield();
    delay(500);
    Serial.print(".");
    now = time(nullptr);
    if (i>60)
    {
      Serial.println("Failed to sync time");
    break;
    }
    i++;
  }
}

void setup() {
  g_isUpdatein =false;
  configObjectStruct co ;
  

  // put your setup code here, to run once:
  Serial.begin(115200);
  pixels.setPixelColor(0, pixels.Color(20, 0, 0));
  pixels.show();
  I2C_0.begin((int) SDApin , SCLpin  );

  while ( !Serial ) delay(100);   // wait for native usb
  reportOutput("Start Display ");
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
     reportOutput(F("SSD1306 allocation failed"),reportLevel::error);    
  }
   // Clear the buffer
  display.clearDisplay();
  display.display();

  displayInformation("Loading");
  reportOutput("Load config from FS ");
  
  if (!LittleFS.begin(true)) {
    //Mount can fail if not formated
    reportOutput("LittleFS mount failed ", reportLevel::error);
  }
  
  // Serial.println("Load certs ");
  // SSLCertStore::getInstance()->loadCertsFromFile("/rootCerts.pem");
  // Serial.println("print cert");
  // Serial.println(SSLCertStore::getInstance()->getCerts());
  
  bool configLoaded = getStoredConfig(&co );
  if ((configLoaded)&&(WiFi.status() != WL_CONNECTED))
  {
    displayInformation("Connect ",co.ssid )    ;
    reportOutput("PW :");
    reportOutput(co.PW);
    connect_wifi(co.ssid, co.PW, MAXWIFICONNECTIONTRY);
  } 

  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Start without wifi");
//  if (connect_wifi(WIFI_SSID, WIFI_PASSWORD, MAXWIFICONNECTIONTRY)>MAXWIFICONNECTIONTRY)
    {
      //NO wifi
      displayInformation("Failed connect Wlan");
      Serial.println("Start hotspot ...");
      WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);
      Serial.println("Start config server ...");
      g_configServer =new wifiConfigServer();
      g_configServer->setupServer();
      return;
    }
  }
 
  Serial.println("Wifi connected");
  displayInformation("Get time");
  setClock();

  if (co.firmwareUpdateState[0]==0xF1)
  {
    co.firmwareUpdateState[0]=0xF2;
    // setStoredConfig(&co);
    // doOTAUpgrade();
    //co.firmwareUpdateState[0]=0xF0;
    //setStoredConfig(&co);
    //ESP.restart();
  }
  else if (co.firmwareUpdateState[0]==0xF2)
  {
    Serial.println("Update failed ");
    // sendUpdateState("Started Old FW after failed update");
    co.firmwareUpdateState[0]==0xF0;
    // setStoredConfig(&co);
  }
  //**************************************
  
  time_t now = time(nullptr);
  //Make sure we have a time, needed for ssl
  if (now < 8 * 3600 * 2)
    return;

  // if ((strlen(co.SCOPE_ID)>1 )&&(strlen(co.DEVICE_ID)>1 )&&(strlen(co.DEVICE_KEY)>1 ))
  //   connect_client(co.SCOPE_ID, co.DEVICE_ID, co.DEVICE_KEY);
 
  // sendRestartReason();
  // sendNewTelemetry();
  g_lastTick = 0;  
 

}

void loop() {
  // put your main code here, to run repeatedly:
g_configServer->loopItem(g_lastTick);
}
