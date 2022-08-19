

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
#include "src/SSLCertStore.h"
#include "src/iotCallbacks.h"
#include "src/globals.h"

// I2C Defintion 
#define SDApin 18
#define SCLpin 19
#define PINVARPORIZER 2
#define PINDEHUMIDIFY 3

#define INTERRUPBUTTONID_DOOR 0
#define INTERRUPBUTTONID_SW1 1
#define INTERRUPBUTTONID_SW2 2

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
#define MAXWIFICONNECTIONTRY 20


#define LEDC_CHANNEL_0     0
#define LEDC_TIMER_12_BIT  12
#define LEDC_BASE_FREQ     5000
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

SSLCertStore* SSLCertStore::_me = 0;

configObjectStruct g_configObject ;
stateStruct currentState;
char g_charBuffer[50];
unsigned long lastButtonPressMS=0;

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
  byte numSsid = WiFi.scanNetworks();
  if (numSsid == -1) {
    LOG_VERBOSE("Couldn't find any WIFI");
  }
  // Serial.print("number of available networks:");
  // Serial.println(numSsid);
  bool foundMyNetwork =false;
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    if (strcmp (WiFi.SSID(thisNet).c_str(),wifi_ssid)==0)
    {
      LOG_VERBOSE("Found Wifi %s",WiFi.SSID(thisNet));
      foundMyNetwork=true;
      break;
    }    
    // Serial.print(WiFi.RSSI(thisNet));
    // Serial.println(" dBm");    // Serial.print("\tEncryption: ");
    // printEncryptionType(WiFi.encryptionType(thisNet));
  }
  if (foundMyNetwork==false )
    return false ;
  if (WiFi.status() != WL_IDLE_STATUS)
    WiFi.disconnect();
  WiFi.begin(wifi_ssid, wifi_password);
  reportOutput("Connecting to WiFi..");
  // Serial.print("|"); Serial.print(wifi_ssid); Serial.println("|");
  // Serial.print("|"); Serial.print(wifi_password); Serial.println("|");
  int countTry=0;
  while (WiFi.status() != WL_CONNECTED) {
    countTry ++;
    reportOutput("Failed try of wifi connection");
    // Serial.println(WiFi.status());
    delay(800);
    if (maxTry< countTry)
      break;
    if (WiFi.status()== WL_CONNECT_FAILED)
    {
      reportOutput("connect wifi Return Connect failed");
    }
  }
  return countTry;
}

void setClock(int maxIterrations =2 ) {
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



void turnOffVaporizer()
{
  digitalWrite(g_configObject.pinVaporizer, LOW); 
  Serial.println("Vaporizer off");  
  if (currentState.vaporizerOn ==true)
  {
    //only change if change is needed
    currentState.vaporizerOn =false;  
    currentState.stateNeedsReport=true;
  }
}

void turnOnVaporizer()
{
  displayInformation("Vaporizer On");
  digitalWrite(g_configObject.pinVaporizer, HIGH);   
  if (currentState.vaporizerOn ==false)
  {
    LOG_VERBOSE("Vaporizer was off");
    //only change if change is needed
    currentState.vaporizerOn =true;  
    currentState.stateNeedsReport=true;
  }
  turnOffHumidifier();  
}

void turnOffHumidifier()
{
  digitalWrite(g_configObject.pinDehumidity, LOW); 
  Serial.println("Dehumidifier off");  
  if (currentState.DehumidifyOn ==true)
  {
    LOG_VERBOSE("Dehumidifier was on");
    //only change if change is needed
    currentState.DehumidifyOn =false;  
    currentState.stateNeedsReport=true;
  }
}

void turnOnDehumidify()
{
  displayInformation("Dehumidifier on") ;  
  digitalWrite(g_configObject.pinDehumidity, HIGH);   
  if (currentState.DehumidifyOn ==false)
  {
    //only change if change is needed
    currentState.DehumidifyOn =true;  
    currentState.stateNeedsReport=true;
  }
  turnOffVaporizer();
}

void TestEnviroment()
{
  currentState.temperature =  g_envSensor->getTemperature();
  currentState.humidity =g_envSensor->getHumidity();   
  //Read Humidity, Temperature and Pressure  
  char text[100];
  sprintf(text,"Temp %2.1fC  Humidity %2.0f\%",currentState.temperature,currentState.humidity);  
  Serial.print(text);
  displayRotation(text) ;
  float targetHumid =(float)g_configObject.targetHumid;
  float tolerance =(float)g_configObject.targetHumidTolerance;
  sprintf(text,"targetHumid : %f  ;tolerance %f   ",targetHumid ,tolerance );
  Serial.print(text);
  sprintf(text,"Minimum : %f  ;Max %f   Haben %f",targetHumid -tolerance,targetHumid +tolerance,currentState.humidity );
  Serial.print(text);
  if( (targetHumid +tolerance) -currentState.humidity <0)
    turnOnDehumidify();
  else if((targetHumid -tolerance)-currentState.humidity  >0)
    turnOnVaporizer ();
  else 
  {
      turnOffVaporizer();
      turnOffHumidifier();
  }

}


bool setupAzure()
{  
  // connect to Azure IoT
  time_t now = time(nullptr);
  if (now < 8 * 3600 * 2)
    return false;
  if (WiFi.status() != WL_CONNECTED)
  {
      if (!connect_wifi(g_configObject.ssid, g_configObject.PW, 2)==2);
        return false;      
  }
  currentState.needReconnectAzure= false;
  if ((strlen(g_configObject.SCOPE_ID)==0) ||
       (strlen(g_configObject.DEVICE_ID)==0) ||
       (strlen(g_configObject.DEVICE_KEY)==0))
       return false;  
  if (!connect_client( g_configObject.SCOPE_ID, g_configObject.DEVICE_ID, g_configObject.DEVICE_KEY))
  {
    g_configServer =new wifiConfigServer();
    g_configServer->setupServer();    
    return false;
  }
  Serial.println("currentState.isAzureConnected in main");
  Serial.println(currentState.isAzureConnected);
  return true ;
}

void IRAM_ATTR doorOpenEventISR(int id) {
  if ( millis() -lastButtonPressMS >300)
  {
    lastButtonPressMS = millis();
    switch (id)
    {
      case INTERRUPBUTTONID_DOOR:
        Serial.println("Door open");
        currentState.isDoorOpen =!currentState.isDoorOpen;  
        reportNewState(&currentState);
        break;
      case INTERRUPBUTTONID_SW1:
        Serial.println("Button 1");
        break;
      case INTERRUPBUTTONID_SW2:
        Serial.println("Button 2");
        break;
    }
    
  }
}
void fanSpeedAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 4095 from 2 ^ 12 - 1
  uint32_t duty = (4095 / valueMax) * min(value, valueMax);
  // write duty to LEDC
  LOG_VERBOSE("FAN speed %d/%d",value,valueMax);
  ledcWrite(channel, duty);
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
  reportOutput("Load config from FS ");
  
  if (!LittleFS.begin(true)) {
    //Mount can fail if not formated
    reportOutput("LittleFS mount failed ", reportLevel::error);
  }
  bool configLoaded = getStoredConfig(&g_configObject );
  
  I2C_0.begin((int) g_configObject.pinSDA , g_configObject.pinSCL  );  
  // while ( !Serial ) delay(100);   // wait for native usb
  reportOutput("Start Display ");
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
     reportOutput(F("SSD1306 allocation failed"),reportLevel::error);    
  }
   // Clear the buffer
  display.clearDisplay();
  display.display();

  displayInformation("Loading");
  
  Serial.println("Load certs ");
  SSLCertStore::getInstance()->loadCertsFromFile("/rootCerts.pem");
  // Serial.println("print cert");
  // Serial.println(SSLCertStore::getInstance()->getCerts());
  
  
  if ((configLoaded)&&(WiFi.status() != WL_CONNECTED))
  {
    displayInformation("Connect ",g_configObject.ssid )    ;
    reportOutput("PW :");
    reportOutput(g_configObject.PW);
    connect_wifi(g_configObject.ssid, g_configObject.PW, MAXWIFICONNECTIONTRY);
  } 
  printConfig(&g_configObject);
  if (WiFi.status() != WL_CONNECTED)
  {    
    
      byte numSsid = WiFi.scanNetworks();
      if (numSsid == -1) {
        Serial.println("Couldn't get a wifi connection");
      }
      Serial.print("number of available networks:");
      Serial.println(numSsid);
      for (int thisNet = 0; thisNet < numSsid; thisNet++) {
        Serial.print(thisNet);
        Serial.print(") ");
        Serial.print(WiFi.SSID(thisNet));
        Serial.print("\tSignal: ");
        Serial.print(WiFi.RSSI(thisNet));
        Serial.println(" dBm");
        // Serial.print("\tEncryption: ");
        // printEncryptionType(WiFi.encryptionType(thisNet));
      }
      //NO wifi
      displayInformation("Failed connect Wlan");
      Serial.println("Start hotspot ...");
      WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASSWORD);   
      g_configServer =new wifiConfigServer();
      g_configServer->setupServer();     
    
  }
  else 
  {
    Serial.println("Wifi connected");
    displayInformation("Get time");
    setClock();
    displayTime();  
    if (g_configObject.firmwareUpdateState[0]==0xF1)
    {
      g_configObject.firmwareUpdateState[0]=0xF2;
      //setStoredConfig(&co);
      //doOTAUpgrade();
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
    // if ((strlen(co.SCOPE_ID)>1 )&&(strlen(co.DEVICE_ID)>1 )&&(strlen(co.DEVICE_KEY)>1 ))
    //   connect_client(co.SCOPE_ID, co.DEVICE_ID, co.DEVICE_KEY);    
    if (!setupAzure())
    {
         g_configServer =new wifiConfigServer();
      g_configServer->setupServer();   
    }
    else 
      iotc_loop();
  }
  
  // sendRestartReason();
  // sendNewTelemetry();

  displayInformation("Setup Sensors");
  
  g_envSensor = new enviroment_SHT31();

  g_envSensor->init(0x44,&I2C_0);

  pinMode(g_configObject.pinVaporizer, OUTPUT);
  pinMode(g_configObject.pinDehumidity, OUTPUT);
  pinMode(g_configObject.pinDoorswitch, INPUT_PULLUP);  
  pinMode(g_configObject.pinControllSwitch1, INPUT_PULLUP);  
  pinMode(g_configObject.pinControllSwitch2, INPUT_PULLUP);  
  attachInterrupt(g_configObject.pinDoorswitch, [](){doorOpenEventISR(INTERRUPBUTTONID_DOOR);}, CHANGE);
  attachInterrupt(g_configObject.pinControllSwitch1, [](){doorOpenEventISR(INTERRUPBUTTONID_SW1);}, FALLING);
  attachInterrupt(g_configObject.pinControllSwitch2, [](){doorOpenEventISR(INTERRUPBUTTONID_SW2);}, FALLING);


  // pinDoorswitch;
  // pinControllSwitch1;
  // pinControllSwitch2;
  // speedFan;

  ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_12_BIT);
  ledcAttachPin(g_configObject.pinFan, LEDC_CHANNEL_0);
  fanSpeedAnalogWrite(LEDC_CHANNEL_0, (uint32_t) g_configObject.speedFan);

  Serial.println("Almost loop time");
  g_lastTick = 0;  
  iotc_loop();

}
int counterTele=0;
 

void loop() {
  
   // set the brightness on LEDC channel 0
  
  if (WiFi.status()==WL_CONNECTION_LOST)
  {
    // We used to be connected but are no longer
    LOG_VERBOSE("Lost Wifi connection");
    connect_wifi(g_configObject.ssid, g_configObject.PW, MAXWIFICONNECTIONTRY);
  }

  unsigned long ms = millis();
  if (ms - g_lastTick > 1000*20) 
  {  
      g_lastTick = ms;
      TestEnviroment();
      if (currentState.stateNeedsReport==true)
      {
        reportNewState(&currentState);
        counterTele=0;
      }
      counterTele++;
      if (counterTele>=60)
      {
        sendNewTelemetry(&currentState);    
        counterTele=0;
      }
  }

  if (g_configServer!= NULL)
  {
    g_configServer->loopItem(g_lastTick);    
    display.fillRect(0,SCREEN_HEIGHT-7,SCREEN_WIDTH,SCREEN_HEIGHT,SSD1306_BLACK);
    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0,SCREEN_HEIGHT-7);             // Start at top-left corner
    IPAddress myIP = WiFi.softAPIP();
    auto t = (ms /10000)%3 ; //this should give two states
    if (t ==1)
      display.println(WiFi.macAddress());
    else if(t==2)
    {
      if (WiFi.status() == WL_CONNECTED)
      {        
        display.println(WiFi.localIP());
      }
      else 
        display.println(WiFi.macAddress());
    }
    else
      display.println(myIP);
    display.display();
  } 
  else 
  {
    displayTime(); // We can not show the time when in config mode.
  }
  if (currentState.isAzureConnected)
  {    
    iotc_loop();  // do background work for iotc
  }
  // else
  //   Serial.println("Not Connected");
  if (currentState.needReconnectAzure)
  {    
    setupAzure();
  }
  delay(1000);
  
}
