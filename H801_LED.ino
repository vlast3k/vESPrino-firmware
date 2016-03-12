#ifdef VTHING_H801_LED

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


int currentPinValue[16];


// RGB FET
#define redPIN    12
#define greenPIN  15
#define bluePIN   13


// W FET
#define w1PIN     14
#define w2PIN     4

// onbaord green LED D1
#define LEDPIN    5
// onbaord red LED D2 // uses SERIAL TX
#define LED2PIN   1

// note 
// TX GPIO2 @SERIAL1 (SERIAL ONE)
// RX GPIO3 @SERIAL    


#define LEDoff digitalWrite(LEDPIN,HIGH)
#define LEDon digitalWrite(LEDPIN,LOW)

#define LED2off digitalWrite(LED2PIN,HIGH)
#define LED2on digitalWrite(LED2PIN,LOW)

// Update these with values suitable for your network.
PubSubClient *h801_mqttClient = NULL;//(server);
IPAddress *mqttIP;//= WiFi.localIP();// = IPAddress(0,0,0,0);
ESP8266WebServer *h801_webServer = NULL;
WiFiClient *wclient;

const char* apiKey = "Qw8rdb20aV";
//http://randomkeygen.com/

void h801_onConfigStored() {
  
}

void h801_processConfig(const char *p) {
  char apiKey[25];
  p = extractStringFromQuotes(p, apiKey, sizeof(apiKey)); 
  putJSONConfig(H801_API_KEY, apiKey);
}

void analogWriteColor(int pin, int value) {
  
  int gamma_table[PWM_VALUE+1] = {
    0, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 8, 9, 10,
    11, 12, 13, 15, 17, 19, 21, 23, 26, 29, 32, 36, 40, 44, 49, 55,
    61, 68, 76, 85, 94, 105, 117, 131, 146, 162, 181, 202, 225, 250,
    279, 311, 346, 386, 430, 479, 534, 595, 663, 739, 824, 918, 1023
  };
  currentPinValue[pin] = value;
  value = map(value,0,100,0,PWM_VALUE);
  value = constrain(value,0,PWM_VALUE);
  value = gamma_table[value];
  SERIAL << "PWM PIN: " << pin << " = " << value << endl;
  analogWrite(pin, value);  
}

void processTriplet(String payload) {
   int c1 = payload.indexOf(';');
   int c2 = payload.indexOf(';',c1+1);
   analogWriteColor(redPIN, payload.toInt());
   analogWriteColor(greenPIN, payload.substring(c1+1,c2).toInt());
   analogWriteColor(bluePIN, payload.substring(c2+1).toInt()); 
}

void stopH801() {
  SERIAL << "on stop" << endl;
  heap("");
  delete h801_webServer;
  delete h801_mqttClient;
  delete wclient;
  heap("");
}

void onh801_HttpRequest() {
  ESP8266WebServer &server = *h801_webServer;
  String apiKey = getJSONConfig(H801_API_KEY);
  if (apiKey && (!server.hasArg("key") || server.arg("key") != apiKey)) {
    server.send(401, "text/plain", "'key' request parameter is required");
    return;
  }

  if (server.hasArg("color")) processTriplet(server.arg("color"));
  if (server.hasArg("sw1")) analogWriteColor(w1PIN, server.arg("sw1").toInt());
  if (server.hasArg("sw2")) analogWriteColor(w2PIN, server.arg("sw2").toInt());
  char sss[100];
  sprintf(sss, "{\"red\":%d,\"green\":%d,\"blue\":%d,\"sw1\":%d,\"sw2\":%d}",
    currentPinValue[redPIN], currentPinValue[greenPIN], currentPinValue[bluePIN], 
    currentPinValue[w1PIN], currentPinValue[w2PIN]);
  server.send(200, "application/json", sss);
}

void h801_webServer_start() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (h801_webServer) delete h801_webServer;
  h801_webServer = new ESP8266WebServer(80);
  h801_webServer->on("/", onh801_HttpRequest); 
  h801_webServer->begin();

  SERIAL.print("\n\nOpen http://");
  SERIAL.print(WiFi.localIP());
  SERIAL.println(F("/ in your browser to see status and /?color=12,15,100&w1=12&w2=56 to set R,G,B,W1 and W2 - range [0,100])"));
  //mqttIP = &WiFi.localIP();
}



void h801_callback(const MQTT::Publish& pub) {
  SERIAL << pub.topic() << " => " << pub.payload_string() << endl;
  String payload = pub.payload_string(), topic = String(pub.topic());
  Serial << topic << ", " << topic.indexOf("/Color") <<" ..." <<endl;
  if      (topic.indexOf("/Color") > -1) processTriplet(payload);
  else if (topic.indexOf("/W1")   > -1) analogWriteColor(w1PIN, payload.toInt());
  else if (topic.indexOf("/W2")   > -1) analogWriteColor(w2PIN, payload.toInt());   
}

void h801_mqtt_connect() {
  if (WiFi.status() != WL_CONNECTED) return;
  if (h801_mqttClient) delete h801_mqttClient;
  char mqttServer[30], mqttClient[20], mqttTopic[40];
  long mqttPort;
  wclient = new WiFiClient();
    //SERIAL << "ADADASdsadasdD" << endl;
  EEPROM.get(EE_MQTT_SERVER_30B, mqttServer);
  if (mqttServer[0] == 0 || mqttServer[0] == 255) return;
  EEPROM.get(EE_MQTT_PORT_4B,    mqttPort);
  EEPROM.get(EE_MQTT_CLIENT_20B, mqttClient);
  EEPROM.get(EE_MQTT_TOPIC_40B,  mqttTopic);

  delay(100);
  SERIAL << "mqtt connecting to: " << mqttServer << " " << mqttPort << " " << mqttClient << endl;
  delay(100);
  h801_mqttClient = new PubSubClient(*wclient, mqttServer, mqttPort);
  h801_mqttClient->set_callback(h801_callback);
  if (h801_mqttClient->connect(mqttClient)) {
    h801_mqttClient->subscribe(String(mqttTopic) + "/Color");
    h801_mqttClient->subscribe(String(mqttTopic) + "/W1");
    h801_mqttClient->subscribe(String(mqttTopic) + "/W2");
    SERIAL.println("MQTT connected");  
    SERIAL << "Subscribed Topics: " << endl << String(mqttTopic) + 
                                  "/Color  Send Payload: R,G,B  from 0-100 (incl), e.g. 15,20,100" << endl <<
              String(mqttTopic) + "/W1     Send Payload: W1  0-100" << endl << String(mqttTopic) + 
                                  "/W2     Send Payload: W1  0-100" << endl;
  }  else {
    SERIAL << " mqtt failed" << endl;
  }
}

void  testH801() {
  boolean res = h801_mqttClient->publish("/openHAB/RGB_2/Color", "12;33;44");
  SERIAL << String(res) << endl;
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
    LED2on;
}

void h801_loop() {
  if (WiFi.status() == WL_CONNECTED) {
    if (h801_mqttClient == NULL) h801_mqtt_connect();
    if (h801_webServer == NULL)  h801_webServer_start();
  }  
  if (h801_mqttClient) h801_mqttClient->loop();
  if (h801_webServer) h801_webServer->handleClient();
 //heap("");
 //delay(500);
}

#endif
