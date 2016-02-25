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
#include <SoftwareSerialESP.h>
#include <Si7021.h>

#include <Timer.h>
#include <RunningAverage.h>
#include <NeoPixelBus.h> 
extern "C" {
#include "user_interface.h"
}

void setSendInterval (const char *line);
void setSendThreshold(const char *line);
void getTS(const char* line);
void sendTS();
void testUBI();
int setWifi(const char* p);
void atCIPSTART(const char *p);
void mockATCommand(const char *line);
void cfgGENIOT(const char *p);
void cfgHCPIOT(const char *p);
void sndIOT(const char *line);
void sndGENIOT(const char *line);
void sndHCPIOT(const char *line);
void addHCPIOTHeaders(HTTPClient *http, const char *token);
void sndSimple();
void configMQTT(const char *p);
void sendMQTT(String msg);
int processResponseCodeATFW(HTTPClient *http, int rc);
int CM1106_read();
void CM1106_init();
void doHttpUpdate(int mode);
void handleOTA();
void startOTA();
char *extractStringFromQuotes(const char* src, char *dest, int destSize);
void storeToEE(int address, const char *str);
void handleWifi();
void connectToWifi(const char *s1, const char *s2, const char *s3);
void wifiScanNetworks();
int wifiConnectToStoredSSID();
void processUserInput();
byte readLine(int timeout);
int handleCommand();
int sendPing();
int httpAuthSAP();
int checkSAPAuth();
void heap(const char * str);
void onTempRead();
void handleSAP_IOT_PushService();
void doSend();
void attachButton();
void processMessage(String payload);
void processCommand(String cmd);
void initLight();

void setSAPAuth(const char *);
char *extractStringFromQuotes(const char* src, char *dest, int destSize=19) ;
#define EE_WIFI_SSID_30B 0
#define EE_WIFI_P1_30B 30
#define EE_WIFI_P2_30B 60
#define EE_IOT_HOST_60B 90
#define EE_IOT_PATH_140B 150
#define EE_IOT_TOKN_40B 290
#define EE_GENIOT_PATH_140B 330
#define EE_MQTT_SERVER_30B  470
#define EE_MQTT_PORT_4B     500
#define EE_MQTT_CLIENT_20B  504
#define EE_MQTT_USER_45B    524
#define EE_MQTT_PASS_15B    569
#define EE_MQTT_TOPIC_40B   584
#define EE_MQTT_VALUE_70B   624
//#define EE_LAST 694

#define DT_VTHING_CO2 1
#define DT_VAIR 2
#define DT_VTHING_STARTER 3

int deviceType = DT_VTHING_STARTER;

String   mqttServer = "m20.cloudmqtt.com";
uint32_t mqttPort   = 19749;
String   mqttClient = "vAir_CO2_Monitor";
String   mqttUser   = "pndhubpk";
String   mqttPass   = "yfT7ax_KDrgG";
String   mqttTopic  = "co2Value";


String mmsHost = "iotmmsi024148trial.hanatrial.ondemand.com";
String deviceId = "e46304a8-a410-4979-82f6-ca3da7e43df9";
String authToken = "8f337a8e54bd352f28c2892743c94b3";
String colors[] = {"red","pink","lila","violet","blue","mblue","cyan","green","yellow","orange"};
#define COLOR_COUNT 10

//h iotmmsi024148trial.hanatrial.ondemand.com
//d c5c73d69-6a19-4c7d-9da3-b32198ba71f9
//m 2023a0e66f76d20f47d7
//v co2
//t 46de4fc404221b32054a8405f602fd

boolean DEBUG = false;

uint32_t intCO2RawRead   =  15000L;
uint32_t intCO2SendValue = 120000L;
uint16_t co2Threshold = 1;
uint32_t lastSentCO2value = 0;

Timer *tmrCO2RawRead, *tmrCO2SendValueTimer, *tmrTempRead;
boolean startedCO2Monitoring = false;
RunningAverage raCO2Raw(4);
NeoPixelBus *strip;// = NeoPixelBus(1, D4);
SI7021 *si7021;

void sendCO2Value() {
  int val = (int)raCO2Raw.getAverage();
  String s = String("sndiot ") + val;
  sndIOT(s.c_str());
  lastSentCO2value = val;
  tmrCO2SendValueTimer->Start();
}

void onCO2RawRead() {
  int res = CM1106_read();
  if (res != 550) startedCO2Monitoring = true;
  if (startedCO2Monitoring) {
    raCO2Raw.addValue(res);
    int diff = raCO2Raw.getAverage() - lastSentCO2value;
    if ((co2Threshold > 0) && (abs(diff) > co2Threshold)) sendCO2Value();
  }
}
char VERSION[] = "vThing - CO2 Monitor v1.0";
void setup() {
  Serial.begin(9600);
//  Serial << "Start Setup: " << millis() << endl;
  switch (deviceType) {
    case DT_VTHING_CO2:  Serial << endl << "vThing - CO2 Monitor v1.1" << endl; break;
    case DT_VAIR:        Serial << endl << "vAir - WiFi Module v1.6.1"   << endl; break;
  }
  EEPROM.begin(1024);
  Serial << "ready" << endl;
  //Serial << "Strip begin: " << millis() << endl;
  //Serial << "Strip end: " << millis() << endl;
  //startWifi();
  Serial << "Waiting for auto-connect" << endl;

  if (deviceType == DT_VTHING_STARTER) {
    strip = new NeoPixelBus(1, D4);
    if (strip) {
      strip->Begin();
      strip->SetPixelColor(0, RgbColor(0, 5,0));
      strip->Show();  
    }    
    si7021 = new SI7021();
    si7021->begin(D1, D6); // Runs : Wire.begin() + reset()
    si7021->setHumidityRes(8); // Humidity = 12-bit / Temperature = 14-bit
    tmrTempRead = new Timer(30000L,   onTempRead);
    tmrTempRead->Start();
    attachButton();
  } else if (deviceType == DT_VTHING_CO2 ) {
    if (strip) {
      strip->Begin();
      strip->SetPixelColor(0, RgbColor(0, 5,0));
      strip->Show();  
    }
    tmrCO2RawRead        = new Timer(intCO2RawRead,   onCO2RawRead);
    tmrCO2SendValueTimer = new Timer(intCO2SendValue, sendCO2Value);
    int res = CM1106_read();
    Serial << "CO2 now: " << res << endl;
    tmrCO2RawRead->Start();
    tmrCO2SendValueTimer->Start();
  }
  //Serial << "Completed Setup: " << millis() << endl;
//WiFi.mode(WIFI_OFF);
  //wifi_set_sleep_type(LIGHT_SLEEP_T);
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

  if (deviceType == DT_VTHING_CO2) {
    tmrCO2RawRead->Update();
    tmrCO2SendValueTimer->Update();
    delay(5000);
  } else if (deviceType == DT_VTHING_STARTER) {
    tmrTempRead->Update();
    handleSAP_IOT_PushService();
    doSend();
    delay(5000);
  }
}
