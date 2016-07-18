#ifndef common_h
#define common_h

#include <NeoPixelBus.h>
#include <CubicGasSensors.h>
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

#define Cred     RgbColor(255, 0, 0)
#define Cpink    RgbColor(255, 0, 128)
#define Clila    RgbColor(255, 0, 255)
#define Cviolet  RgbColor(128, 0, 255)
#define Cblue    RgbColor(0, 0, 255)
#define Cmblue   RgbColor(0, 128, 255)
#define Ccyan    RgbColor(0, 255, 255)
#define Cgreen   RgbColor(0, 255, 0)
#define Cyellow  RgbColor(255, 255, 0)
#define Corange  RgbColor(255, 100, 0)
#define Cwhite   RgbColor(255, 255, 255)
#define Cblack   RgbColor(0)

#define SERIAL Serial
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
#define EE_1B_RESET_CO2     694
//#define EE_LAST 695
#define EE_JSON_CFG_1000B   1000

#define SAP_IOT_HOST "spHst"
#define SAP_IOT_DEVID "spDvId"
#define SAP_IOT_TOKEN "spTok"
#define SAP_IOT_BTN_MSGID "spBtMID"
#define H801_API_KEY "h801key"
#define XX_SND_INT  "xxSndInt"
#define XX_SND_THR  "xxSndThr"

void heap(const char * str);
void setSendInterval (const char *line);
void setSendThreshold(const char *line);
void getTS(const char* line);
void sendTS();
void testUBI();
int setWifi(const char* p);
void atCIPSTART(const char *p);
void mockATCommand(const char *line);
void cfgGENIOT(const char *p);
void cfgHCPIOT1(const char *p);
void cfgHCPIOT2(const char *p);
void sndIOT(const char *line);
void sndGENIOT(const char *line);
void sndHCPIOT(const char *line);
void addHCPIOTHeaders(HTTPClient *http, const char *token);
void sndSimple();
void configMQTT(const char *p);
void sendMQTT(String msg);
int processResponseCodeATFW(HTTPClient *http, int rc);
void doHttpUpdate(int mode, char *url);
char *extractStringFromQuotes(const char* src, char *dest, int destSize);
void storeToEE(int address, const char *str, int maxLength);
void handleWifi();
void connectToWifi(const char *s1, const char *s2, const char *s3);
void wifiScanNetworks();
int wifiConnectToStoredSSID();
boolean processUserInput();
byte readLine(int timeout);
int handleCommand();
void heap(const char * str);
void processMessage(String payload);
void processCommand(String cmd);
void initLight();
void printJSONConfig();
void putJSONConfig(const char *key, int value, boolean commit = true);
void putJSONConfig(const char *key, const char *value, boolean isArrayValue = false, boolean commit = true);
void dumpTemp();
void factoryReset();
void activeWait();
char *getJSONConfig(const char *item, char *buf, char *p1 = NULL, char *p3=NULL);
void testJSON();
void testHttpUpdate();
void setSAPAuth(const char *);
char *extractStringFromQuotes(const char* src, char *dest, int destSize=19) ;
void scani2c();
void printVersion();
void handleCommandVESPrino(char *line);
byte readLine(int timeout);
int handleCommand();
void sendPingPort(const char *p);
void oledHandleCommand(char *cmd);
void loopNeoPixel();
void checkButtonSend();

//#endif

//#ifdef VTHING_STARTER
boolean si7021init();
void onTempRead();
void handleSAP_IOT_PushService();
void doSend();
void attachButton();
void initVThingStarter();
void loopVThingStarter();

void handleCommandVESPrino(char *line);

//#endif

#ifdef VTHING_H801_LED
void stopH801();
void testH801();
void h801_setup();
void h801_loop();
void h801_onConfigStored();
void h801_mqtt_connect();
void h801_processConfig(const char *p);
#endif
#ifdef VTHING_CO2

void initCO2Handler();
void loopCO2Handler();


#endif
extern boolean hasSSD1306, hasSI7021, hasPN532, hasBMP180, hasBH1750, hasAPDS9960;
extern bool shouldSend;
extern boolean DEBUG;
extern boolean SKIP_LOOP;
#endif
