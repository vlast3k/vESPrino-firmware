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

#include <Timer.h>
#include <RunningAverage.h>
#include <NeoPixelBus.h> 

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

String   mqttServer = "m20.cloudmqtt.com";
uint32_t mqttPort   = 19749;
String   mqttClient = "vAir_CO2_Monitor";
String   mqttUser   = "pndhubpk";
String   mqttPass   = "yfT7ax_KDrgG";
String   mqttTopic  = "co2Value";

//h iotmmsi024148trial.hanatrial.ondemand.com
//d c5c73d69-6a19-4c7d-9da3-b32198ba71f9
//m 2023a0e66f76d20f47d7
//v co2
//t 46de4fc404221b32054a8405f602fd

uint32_t intCO2RawRead   =  15000L;
uint32_t intCO2SendValue = 120000L;
uint16_t co2Threshold = 1;
uint32_t lastSentCO2value = 0;

Timer *tmrCO2RawRead, *tmrCO2SendValueTimer;
boolean startedCO2Monitoring = false;
RunningAverage raCO2Raw(4);
NeoPixelBus strip = NeoPixelBus(1, D4);

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
    Serial << "CO2: " << raCO2Raw.getAverage()<< " , last: " << lastSentCO2value  << " , " << millis() /1000 <<  endl;
    int diff = raCO2Raw.getAverage() - lastSentCO2value;
    if ((co2Threshold > 0) && (abs(diff) > co2Threshold)) sendCO2Value();
  }
}

void setup() {
  Serial.begin(115200L);
  Serial << "Start Setup: " << millis() << endl;
  Serial << "vThing - CO2 Monitor v0.1" << endl;
  EEPROM.begin(1024);
  Serial << endl << "ready" << endl;
  Serial << "Strip begin: " << millis() << endl;
  strip.Begin();
  strip.SetPixelColor(0, RgbColor(0, 5,0));
  strip.Show();  
  Serial << "Strip end: " << millis() << endl;
  //startWifi();
  Serial << "Waiting for auto-connect" << endl;
  tmrCO2RawRead        = new Timer(intCO2RawRead,   onCO2RawRead);
  tmrCO2SendValueTimer = new Timer(intCO2SendValue, sendCO2Value);
  int res = CM1106_read();
  Serial << "CO2 now: " << res << endl;
  tmrCO2RawRead->Start();
  tmrCO2SendValueTimer->Start();
  Serial << "Completed Setup: " << millis() << endl;
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

  tmrCO2RawRead->Update();
  tmrCO2SendValueTimer->Update();
  delay(200);
}
