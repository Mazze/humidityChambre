

#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>
#include <WiFi.h>
#include <Adafruit_BME280.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_BME280.h>
// #include "Adafruit_SHT31.h"
#include <FS.h>
#include <LittleFS.h>

#include "src/configObjectStruct.h"
#include "src/Server/wifiConfigServer.h"
#include "src/logging.h"

#include "src/Sensors/enviromentSensorInterface.h"
#include "src/Sensors/enviroment_SHT31.h"
#include "src/Sensors/enviroment_BME280.h"
// I2C Defintion 
#define SDApin 18
#define SCLpin 19
#define PINVARPORIZER 2
#define PINDEHUMIDIFY 3
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

// Adafruit_BME280 g_sensorBme280;
enviromentSensorInterface* g_envSensor;
// Adafruit_SHT31 g_sensorSht31 =NULL;
// enviroment_SHT31 g_envSht31;
using namespace logging;

bool g_isUpdatein = false;
unsigned long g_lastTick = 0;
wifiConfigServer* g_configServer=NULL;
hw_timer_t * g_timeoutDisplay = NULL;
hw_timer_t * g_timeoutAction = NULL;
//list<String> listOfErrors()

struct stateStruct{

  bool vaporizerOn = false;
  bool DehumidifyOn = false;
  float temperature = -100;
  float humidity =-1;
} currentState;

configObjectStruct g_configObject ;

char g_charBuffer[50];


void setClock(int );

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
  display.fillRect(0,0,SCREEN_WIDTH,8,SSD1306_BLACK);
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,0);             // Start at top-left corner
  display.println(g_charBuffer);
  Serial.println(g_charBuffer);
  timerWrite(g_timeoutDisplay, 0);
  
  display.display();
  
  // Start an alarm
  timerAlarmEnable(g_timeoutDisplay);  
}


void ARDUINO_ISR_ATTR  onDisplayTimeout()
{
  display.fillRect(0,0,SCREEN_WIDTH,8,SSD1306_BLACK);
}

void displayTime() 
{
  time_t now = time(nullptr);
  if (now < 8 * 3600 * 2)
  {
    setClock(1);      
  }
  strftime(g_charBuffer, 20, "%H:%M:%S %m-%d", localtime(&now));  
  display.fillRect(0,SCREEN_HEIGHT-7,SCREEN_WIDTH,SCREEN_HEIGHT,SSD1306_BLACK);
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,SCREEN_HEIGHT-7);             // Start at top-left corner
  display.println(g_charBuffer);
  display.display();
}

void displayRotation(char * text) 
{
  
  
  display.fillRect(0,9,SCREEN_WIDTH,SCREEN_HEIGHT-9,SSD1306_BLACK);
  display.setTextSize(1);             // Normal 1:1 pixel scale
  display.setTextColor(SSD1306_WHITE);        // Draw white text
  display.setCursor(0,9);             // Start at top-left corner
  display.println(text);
  display.display();
  display.startscrollright(9, 26);
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

void setClock(int maxIterrations =4 ) {
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
    
    if (i>maxIterrations*32)
    {
      Serial.println("Failed to sync time");
    break;
    }
    else if((i%32)==31 )
    {
      Serial.println("\nRetry");
      configTime(0, 0, co.ntpserver1, co.ntpserver2);  // UTC  
    }
    i++;
  }
  // configTime(0, 0,co.ntpserver2, co.ntpserver1);
}

void   turnOffVaporizer()
{
  digitalWrite(PINVARPORIZER, LOW); 
  Serial.println("off t");
  currentState.vaporizerOn =false;
  //displayInformation("Vaporizer Off") ;
}

void turnOnVaporizer()
{
  displayInformation("Vaporizer On")    ;
  digitalWrite(PINVARPORIZER, HIGH); 
  currentState.vaporizerOn =true;  
}

void turnOffHumidifier()
{
  digitalWrite(PINDEHUMIDIFY, LOW); 
  Serial.println("off t");  
  currentState.DehumidifyOn =false;
  //displayInformation("Vaporizer Off") ;
}

void turnOnDehumidify()
{
  displayInformation("Dehumidifier on") ;
  turnOffVaporizer();
  digitalWrite(PINDEHUMIDIFY, HIGH); 
  currentState.DehumidifyOn = true;  
}

void TestEnviroment()
{
  currentState.temperature =  g_envSensor->getTemperature();
  currentState.humidity =g_envSensor->getHumidity();   
  //Read Humidity, Temperature and Pressure  
  char text[30];
  sprintf(text,"Temp %2.1fC  Humidity %2.0f\%",currentState.temperature,currentState.humidity);
  Serial.print(text);
  displayRotation(text) ;
  float targetHumid = g_configObject.targetHumid;
  float tolerance =g_configObject.targetHumidTolerance;
  if(currentState.humidity  - targetHumid -tolerance >0)
    turnOnDehumidify();
  else if(currentState.humidity  - targetHumid +tolerance <0)
    turnOnVaporizer ();
  else 
  {
      turnOffVaporizer();
      turnOffHumidifier();
  }

}

void setup() {
  g_isUpdatein =false;
  
  
  g_timeoutDisplay =timerBegin(0, 80, true);
  g_timeoutAction =timerBegin(1, 80, true);
  timerAlarmWrite(g_timeoutDisplay, 10000000, false);

  // Attach onTimer function to our timer.
  timerAttachInterrupt(g_timeoutDisplay, &onDisplayTimeout, true);

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
  
  bool configLoaded = getStoredConfig(&g_configObject );
  if ((configLoaded)&&(WiFi.status() != WL_CONNECTED))
  {
    displayInformation("Connect ",g_configObject.ssid )    ;
    reportOutput("PW :");
    reportOutput(g_configObject.PW);
    connect_wifi(g_configObject.ssid, g_configObject.PW, MAXWIFICONNECTIONTRY);
  } 

  if (WiFi.status() != WL_CONNECTED)
  {    
//  if (connect_wifi(WIFI_SSID, WIFI_PASSWORD, MAXWIFICONNECTIONTRY)>MAXWIFICONNECTIONTRY)
    {
      //NO wifi
      displayInformation("Failed connect Wlan");
      Serial.println("Start hotspot ...");
      WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);   
      g_configServer =new wifiConfigServer();
      g_configServer->setupServer();     
    }
  }
  else 
  {
    Serial.println("Wifi connected");
    displayInformation("Get time");
    setClock();
    displayTime();
    displayInformation("Time : ?????");
    if (g_configObject.firmwareUpdateState[0]==0xF1)
    {
      g_configObject.firmwareUpdateState[0]=0xF2;
      // setStoredConfig(&co);
      // doOTAUpgrade();
      //co.firmwareUpdateState[0]=0xF0;
      //setStoredConfig(&co);
      //ESP.restart();
    }
    else if (g_configObject.firmwareUpdateState[0]==0xF2)
    {
      Serial.println("Update failed ");
      // sendUpdateState("Started Old FW after failed update");
      g_configObject.firmwareUpdateState[0]==0xF0;
      // setStoredConfig(&co);
    }
    //**************************************
    
    
    //Make sure we have a time, needed for ssl
    // if (now < 8 * 3600 * 2)
    //   return;

    // if ((strlen(co.SCOPE_ID)>1 )&&(strlen(co.DEVICE_ID)>1 )&&(strlen(co.DEVICE_KEY)>1 ))
    //   connect_client(co.SCOPE_ID, co.DEVICE_ID, co.DEVICE_KEY);
  }
  // sendRestartReason();
  // sendNewTelemetry();

  displayInformation("Setup Sensors");
  
  g_envSensor = new enviroment_SHT31();

  g_envSensor->init(0x44,&I2C_0);

  pinMode(PINVARPORIZER, OUTPUT);
  pinMode(PINDEHUMIDIFY, OUTPUT);
  
  Serial.println("Almost loop time");
  g_lastTick = 0;  
 

}

void loop() {
  
  unsigned long ms = millis();
  if (ms - g_lastTick > 1000*20) 
  {  
      g_lastTick = ms;
      TestEnviroment();
  }

  if (g_configServer!= NULL)
    g_configServer->loopItem(g_lastTick);
  else 
    displayTime(); // We can not show the time when in config mode.
  
  
  delay(1000);
  
}
