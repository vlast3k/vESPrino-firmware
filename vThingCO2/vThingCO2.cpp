#include <Arduino.h>
#include <NeoPixelBus.h>
#include <CubicGasSensors.h>
//H801 build with 1mb / 256k
//#define VTHING_H801_LED
#define VTHING_STARTER
//#define VAIR
//#define VTHING_CO2

#include "common.hpp"



using namespace std;

#include <Streaming.h>
#include <EEPROM.h>
#include <Math.h>
#include <algorithm>    // std::min
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <Timer.h>
extern "C" {
#include "user_interface.h"
}
#ifdef ESP8266
  #include <SoftwareSerialESP.h>
#else
  #include <SoftwareSerial.h>
#endif
  Timer *tmrStopLED;


// void setSendInterval (const char *line);
// void setSendThreshold(const char *line);
// void getTS(const char* line);
// void sendTS();
// void testUBI();
// int setWifi(const char* p);
// void atCIPSTART(const char *p);
// void mockATCommand(const char *line);
// void cfgGENIOT(const char *p);
// void cfgHCPIOT1(const char *p);
// void cfgHCPIOT2(const char *p);
// void sndIOT(const char *line);
// void sndGENIOT(const char *line);
// void sndHCPIOT(const char *line);
// void addHCPIOTHeaders(HTTPClient *http, const char *token);
// void sndSimple();
// void configMQTT(const char *p);
// void sendMQTT(String msg);
// int processResponseCodeATFW(HTTPClient *http, int rc);
// void doHttpUpdate(int mode, char *url);
// char *extractStringFromQuotes(const char* src, char *dest, int destSize);
// void storeToEE(int address, const char *str, int maxLength);
// void handleWifi();
// void connectToWifi(const char *s1, const char *s2, const char *s3);
// void wifiScanNetworks();
// int wifiConnectToStoredSSID();
// boolean processUserInput();
// byte readLine(int timeout);
// int handleCommand();
// void heap(const char * str);
// void processMessage(String payload);
// void processCommand(String cmd);
// void initLight();
// void printJSONConfig();
// void putJSONConfig(const char *key, int value, boolean commit = true);
// void putJSONConfig(const char *key, const char *value, boolean isArrayValue = false, boolean commit = true);
// char *getJSONConfig(const char *item, char *buf, char *p1 = NULL, char *p3=NULL);
// void dumpTemp();
// void factoryReset();
// void activeWait();
//
// void testJSON();
// void testHttpUpdate();
// void setSAPAuth(const char *);
// char *extractStringFromQuotes(const char* src, char *dest, int destSize=19) ;
// void scani2c();

// #ifdef VTHING_H801_LED
// void stopH801();
// void testH801();
// void h801_setup();
// void h801_loop();
// void h801_onConfigStored();
// void h801_mqtt_connect();
// void h801_processConfig(const char *p);
// #endif



// #ifdef VTHING_CO2
//
// void initCO2Handler();
// void loopCO2Handler();
//
//
// #endif

NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod>  *strip;// = NeoPixelBus(1, D4);

#ifdef VTHING_H801_LED
  #define SERIAL Serial1
#else
  #include <RunningAverage.h>
  #include <NeoPixelBus.h>
  #include <vSAP_Auth.h>

  #define SERIAL Serial



  void onStopLED() {
   // SERIAL << "STOP LED Executed " << endl;
      strip->SetPixelColor(0, RgbColor(0, 0,0));
      strip->Show();
  }

#endif
int pgpio0, pgpio2;

String VERSION = "v1.16";
void printVersion() {
  SERIAL << endl;
  #ifdef VTHING_CO2
    SERIAL << F("vThing - CO2 Monitor ");
  #elif defined(VTHING_STARTER)
    SERIAL << F("vThing - Starter Edition ");
  #elif defined(VTHING_H801_LED)
    SERIAL << F("vThing - H801 FW ");
  #elif defined(VAIR)
    SERIAL << F("vAir - WiFi Module ");
  #endif
  SERIAL << VERSION << endl;
  SERIAL << F("IP address: ") << WiFi.localIP() << endl;
}


void setup() {
  SERIAL.begin(9600);

  //WiFi.begin();
  #if defined(VTHING_CO2) || defined(VTHING_STARTER)
  SERIAL << "AAAA";
    strip = new NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> (1, D4);
    strip->Begin();
    strip->SetPixelColor(0, RgbColor(20, 0, 10));
    strip->Show();
  #endif

  //Serial.begin(9600);
  //Serial1.begin(9600);
  //pinMode(0, INPUT);
  //pinMode(2, INPUT);
  //pgpio0 = digitalRead(0);
  //pgpio2 = digitalRead(2);
  printVersion();
  EEPROM.begin(3000);
  SERIAL << F("ready") << endl;

  SERIAL << F("Waiting for auto-connect") << endl;
  activeWait();

  #ifdef VTHING_STARTER
    initVThingStarter();
  #elif defined(VTHING_CO2)
    initCO2Handler();
  #elif defined(VTHING_H801_LED)
    h801_setup();
  #endif
}

void loop() {
  handleWifi();
  while (processUserInput()) delay(1000);
  if (SKIP_LOOP) {delay(100); return;}

  #ifdef VTHING_CO2
    loopCO2Handler();
  #elif defined(VTHING_STARTER)
    loopVThingStarter();
  #elif defined(VTHING_H801_LED)
    h801_loop();
  #endif
}
