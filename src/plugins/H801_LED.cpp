
#ifdef VTHING_H801_LED
#include <Arduino.h>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>


// #define PWMRANGE 255
// #define PWM_VALUE 31
//int gamma_table[PWM_VALUE+1] = {
//    0, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 8, 10, 11, 13, 16, 19, 23,
//    27, 32, 38, 45, 54, 64, 76, 91, 108, 128, 152, 181, 215, 255
//}; 

 #define PWM_VALUE 63
//int gamma_table[PWM_VALUE+1] = {
//    0, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 5, 5, 6, 6, 7, 8, 9, 10,
//    11, 12, 13, 15, 17, 19, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55,
//    61, 68, 76, 85, 94, 105, 117, 131, 146, 162, 181, 202, 225, 250,
//    279, 311, 346, 386, 430, 479, 534, 595, 663, 739, 824, 918, 1023
//};


float currentPinValue[16];


// RGB FET
//#define redPIN    1215
//#define greenPIN  1513
//#define bluePIN   1312
#define redPIN    15
#define greenPIN  13
#define bluePIN   12


// W FET
#define w1PIN     14
#define w2PIN     4

// onbaord green LED D1
#define LEDPIN    5
// onbaord red LED D2 // uses LOGGER TX
#define LED2PIN   1

// note
// TX GPIO2 @SERIAL1 (LOGGER ONE)
// RX GPIO3 @LOGGER


#define LEDoff digitalWrite(LEDPIN,HIGH)
#define LEDon digitalWrite(LEDPIN,LOW)

#define LED2off digitalWrite(LED2PIN,HIGH)
#define LED2on digitalWrite(LED2PIN,LOW)

// Update these with values suitable for your network.
PubSubClient *h801_mqttClient = NULL;//(server);
IPAddress *mqttIP;//= WiFi.localIP();// = IPAddress(0,0,0,0);
ESP8266WebServer *h801_webServer = NULL;
WiFiClient *wclient;
boolean h801led = false;
int h801ConnectRetries = 0;
Timer *tmr1, *tmrStoreData, *tmrH801mqttKeepAlive;

const char* apiKey = "Qw8rdb20aV";
//http://randomkeygen.com/

void h801_onConfigStored() {

}

void h801_processConfig(const char *p) {
  char apiKey[25];
  p = extractStringFromQuotes(p, apiKey, sizeof(apiKey));
  putJSONConfig(H801_API_KEY, apiKey);
}

void onH801LEDStoreData() {
  LOGGER << "H801 Storing LED configuration\n";
  putJSONConfig("R", currentPinValue[redPIN], false, false);
  putJSONConfig("G", currentPinValue[greenPIN], false, false);
  putJSONConfig("B", currentPinValue[bluePIN], false, false);
  putJSONConfig("W1", currentPinValue[w1PIN], false, false);
  putJSONConfig("W2", currentPinValue[w2PIN], false, true);
}

void analogWriteColor(int pin, float value, boolean startStoreTimer = false) {
//  int gamma_table[PWM_VALUE+1] = {
//    0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 8, 9, 10,
//    11, 12, 13, 15, 17, 19, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55,
//    61, 68, 76, 85, 94, 105, 117, 131, 146, 162, 181, 202, 225, 250,
//    279, 311, 346, 386, 430, 479, 534, 595, 663, 739, 824, 918, 1023
//  };
  currentPinValue[pin] = value;
  value = map(value,0,100,0,1023);
  value = constrain(value,0,1023);
  //value = gamma_table[value];
  if (DEBUG) LOGGER << "PWM PIN: " << pin << " = " << value << endl;
  analogWrite(pin, value);
  //if (startStoreTimer) tmrStoreData->Start();
}

void h801Fade(int r, int g, int b) {
  int timeMS = 2000;
  int stepsPerSec = 20;
  int steps = timeMS * stepsPerSec / 1000;
  int msPerStep = 1000 / stepsPerSec;
  if (DEBUG) LOGGER << "steps : " << steps << endl;
  float rStep = ((float)(currentPinValue[redPIN]   - r)) / steps;
  float gStep = ((float)(currentPinValue[greenPIN] - g)) / steps;
  float bStep = ((float)(currentPinValue[bluePIN]  - b)) / steps;

  for (int i=0; i < steps -1; i++) {
    if (DEBUG) LOGGER << "rstep: " << rStep << ", " << currentPinValue[redPIN]     - rStep << endl;
    analogWriteColor(redPIN, currentPinValue[redPIN]     - rStep, false);
    analogWriteColor(greenPIN, currentPinValue[greenPIN] - gStep, false);
    analogWriteColor(bluePIN, currentPinValue[bluePIN]   - bStep, false);
    delay(msPerStep);
  }
  analogWriteColor(redPIN,   r, false);
  analogWriteColor(greenPIN, g, false);
  analogWriteColor(bluePIN,  b, true);

}

void processTriplet(String payload) {
  int r = currentPinValue[redPIN], g = currentPinValue[greenPIN], b = currentPinValue[bluePIN];
  int c2, c1 = payload.indexOf(';');
  if (c1 > -1) c2 = payload.indexOf(';',c1+1);
  r = payload.toInt();
  if (c1 > -1 && c2 > -1) {
    g = payload.substring(c1+1,c2).toInt();
    b = payload.substring(c2+1).toInt();
  }
  h801Fade(r, g, b);
}

void stopH801() {
  LOGGER << "on stop" << endl;
  heap("");
  delete h801_webServer;
  delete h801_mqttClient;
  delete wclient;
  heap("");
}

String h801LEDConfigAsJSON() {
  char sss[100];
  sprintf(sss, "{\"red\":%d,\"green\":%d,\"blue\":%d,\"w1\":%d,\"w2\":%d,\"ver\":\"%s\",\"http\":%d}",
    (int)currentPinValue[redPIN], (int)currentPinValue[greenPIN], (int)currentPinValue[bluePIN],
    (int)currentPinValue[w1PIN], (int)currentPinValue[w2PIN], VERSION.c_str(), 1);//(h801_webServer)?h801_webServer->getServerStatus():44);
  return String(sss);
}

void publishMQTTStatus() {
  if (WiFi.status() == WL_CONNECTED && h801_mqttClient->connected()) {
    char mqttTopic[40];
    EEPROM.get(EE_MQTT_TOPIC_40B,  mqttTopic);
    h801_mqttClient->publish((String(mqttTopic) + "/Status").c_str(), h801LEDConfigAsJSON().c_str(), true);
  }
}

void onh801_HttpRequest() {
  ESP8266WebServer &server = *h801_webServer;
  char tmp[30];
  String apiKey = String(getJSONConfig(H801_API_KEY, tmp));
  if (apiKey && (!server.hasArg("key") || server.arg("key") != apiKey)) {
    server.send(401, "text/plain", "'key' request parameter is required");
    return;
  }

  if (server.hasArg("color")) processTriplet(server.arg("color"));
  if (server.hasArg("sw1")) analogWriteColor(w1PIN, server.arg("sw1").toInt());
  if (server.hasArg("sw2")) analogWriteColor(w2PIN, server.arg("sw2").toInt());
  if (server.hasArg("save")) onH801LEDStoreData();


  server.send(200, "application/json", h801LEDConfigAsJSON().c_str());
  publishMQTTStatus();
}

void h801_webServer_start() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (h801_webServer) delete h801_webServer;
  h801_webServer = new ESP8266WebServer(80);
  h801_webServer->on("/", onh801_HttpRequest);
  h801_webServer->begin();

  LOGGER << F("\n\nOpen http://") << WiFi.localIP() << F("/ in your browser to see status and\nhttp://") <<
                                     WiFi.localIP() << F("/?color=12;15;100&w1=12&w2=56 to set R,G,B,W1 and W2 - range [0,100])\nadd 'save=true' param, to store data\n");
}


//void h801_callback(const MQTT::Publish& pub) {
void h801_callback(char* topic1, byte* payload1, unsigned int length) {
  char bPayload[60];
  strncpy(bPayload, (char*)payload1, _min(length, sizeof(bPayload)-1));
  bPayload[length] = 0;
  String payload = String((char*)bPayload), topic = String(topic1);
  LOGGER << topic << " => " << payload << endl;
  if      (topic.indexOf("/Color") > -1 && !isDigit(payload1[0])) publishMQTTStatus();
  else if (topic.indexOf("/Color") > -1) processTriplet(payload);
  else if (topic.indexOf("/W1")   > -1) analogWriteColor(w1PIN, payload.toInt());
  else if (topic.indexOf("/W2")   > -1) analogWriteColor(w2PIN, payload.toInt());
  else if (topic.indexOf("/Save") > -1) onH801LEDStoreData();
  //publishMQTTStatus();
}

void h801_mqtt_connect() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (h801_mqttClient) delete h801_mqttClient;
  if (wclient) delete wclient;
  char mqttServer[30], mqttClient[20], mqttTopic[40];
  long mqttPort;
  wclient = new WiFiClient();
    //LOGGER << "ADADASdsadasdD" << endl;
  EEPROM.get(EE_MQTT_SERVER_30B, mqttServer);
  if (mqttServer[0] == 0 || mqttServer[0] == 255) return;
  EEPROM.get(EE_MQTT_PORT_4B,    mqttPort);
  EEPROM.get(EE_MQTT_CLIENT_20B, mqttClient);
  EEPROM.get(EE_MQTT_TOPIC_40B,  mqttTopic);

  delay(100);
  LOGGER << "mqtt connecting to: " << mqttServer << " " << mqttPort << " " << mqttClient << endl;
  delay(100);
  h801_mqttClient = new PubSubClient(mqttServer, mqttPort, h801_callback, *wclient);
//  h801_mqttClient->set_callback(h801_callback);
  if (h801_mqttClient->connect(mqttClient)) {
    h801_mqttClient->subscribe((String(mqttTopic) + "/Color").c_str());
    h801_mqttClient->subscribe((String(mqttTopic) + "/W1").c_str());
    h801_mqttClient->subscribe((String(mqttTopic) + "/W2").c_str());
    h801_mqttClient->subscribe((String(mqttTopic) + "/Save").c_str());
    LOGGER.println("MQTT connected");
    LOGGER << F("Subscribed Topics: \n") <<
                mqttTopic << F("/Color  Send Payload: R;G;B  from 0-100 (incl), e.g. 15;20;100\n") <<
                mqttTopic << F("/W1     Send Payload: W1  0-100\n") <<
                mqttTopic << F("/W2     Send Payload: W1  0-100\n") <<
                mqttTopic << F("/Save   to store settings to flash (LEDs will flash once)\n") <<
                F("Status update will be published on: ") << mqttTopic << "/Status\n";
  }  else {
    LOGGER << " mqtt failed" << endl;
  }
}

void testH801(const char *ignore) {
  boolean res = h801_mqttClient->publish("/openHAB/RGB_2/Color", "12;33;44");
  LOGGER << String(res) << endl;
}

void onH801hb() {
  if (h801led) LED2off;
  else LED2on;
  h801led = !h801led;
  //LOGGER << tmr1->getInterval() << endl;
  if (WiFi.status() != WL_CONNECTED) {
    tmr1->setInterval(100);
    LEDon;
    //LOGGER << "nowifi" << endl;
  } else if(!h801_mqttClient->connected()) {
    if (h801ConnectRetries > 30) {
      h801_mqtt_connect();
      h801ConnectRetries = 0;
    } else {
      tmr1->setInterval(500);
      LEDon;
      h801ConnectRetries++;
    }
  } else {
    tmr1->setInterval(1000);
    LEDoff;
    //h801_mqttClient->publish("h801/status", "OK");
  }
}



void loadLEDDataH801() {
  uint32_t x = millis();
  char tmp[30];
  analogWriteColor(redPIN,   String(getJSONConfig("R", tmp)).toInt(), false);
  analogWriteColor(greenPIN, String(getJSONConfig("G", tmp)).toInt(), false);
  analogWriteColor(bluePIN,  String(getJSONConfig("B", tmp)).toInt(), false);
  analogWriteColor(w1PIN,    String(getJSONConfig("W1", tmp)).toInt(), false);
  analogWriteColor(w2PIN,    String(getJSONConfig("W2", tmp)).toInt(), false);
  x = millis() - x;
  LOGGER << F("LED Config Loaded in: ") << x << F(" ms\n");

  LOGGER << F("Loaded LED Config:") << h801LEDConfigAsJSON();
}

void h801_setup() {
  pinMode(LEDPIN, OUTPUT);
  pinMode(LED2PIN, OUTPUT);

  pinMode(redPIN, OUTPUT);
  pinMode(greenPIN, OUTPUT);
  pinMode(bluePIN, OUTPUT);
  pinMode(w1PIN, OUTPUT);
  pinMode(w2PIN, OUTPUT);

  LEDoff;
  LED2off;
  tmr1 = new Timer(1000, onH801hb);
  tmr1 -> Start();
  tmrH801mqttKeepAlive = new Timer(120L * 1000, publishMQTTStatus);
  tmrH801mqttKeepAlive->Start();
//  tmrStoreData = new Timer(5000, onH801LEDStoreData, true);

  loadLEDDataH801();
}

void h801_loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (h801_mqttClient == NULL) h801_mqtt_connect();
    if (h801_webServer == NULL)  h801_webServer_start();
  }
  if (h801_mqttClient) h801_mqttClient->loop();
  if (h801_webServer) h801_webServer->handleClient();
  tmr1->Update();
  tmrH801mqttKeepAlive->Update();
//  tmrStoreData->Update();
 //heap("");
  delay(100);
}

void H801_registerCommands(menuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("testled"), CMD_EXACT, testH801 , F("")));
  handler->registerCommand(new MenuEntry(F("h801cfg"), CMD_BEGIN, h801_processConfig, F("")));
}

#endif
