#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <Streaming.h>
#include <EEPROM.h>
#include <Math.h>
#include <algorithm>    // std::min
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>

void setSAPAuth(const char *);
char *extractStringFromQuotes(const char* src, char *dest, int destSize=19) ;
#define EE_WIFI_SSID_30B 0
#define EE_WIFI_P1_30B 30
#define EE_WIFI_P2_30B 60
#define EE_IOT_HOST_60B 90
#define EE_IOT_PATH_140B 150
#define EE_IOT_TOKN_40B 290
#define EE_GENIOT_PATH_140B 330
//#define EE_LAST 470

//h iotmmsi024148trial.hanatrial.ondemand.com
//d c5c73d69-6a19-4c7d-9da3-b32198ba71f9
//m 2023a0e66f76d20f47d7
//v co2
//t 46de4fc404221b32054a8405f602fd

void setup() {
  Serial.begin(9600);
  Serial << "vThing - CO2 Monitor v0.1" << endl;
  EEPROM.begin(1024);
  Serial << endl << "ready" << endl;
  //startWifi();
  Serial << "Waiting for auto-connect" << endl;
    //WiFi.begin("vladiHome", "0888414447");
  
//  while (WiFi.status() != WL_CONNECTED) {
//    delay(500);
//    Serial.print(".");
//  }
}


boolean startedOTA = false;
void loop() {
  handleWifi();
  handleOTA();
  if (!startedOTA) {
    processUserInput();
  }
}
