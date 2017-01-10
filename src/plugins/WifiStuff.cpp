#include "common.hpp"
#include "plugins/SAP_HCP_IOT_Plugin.hpp"
#include "plugins/CustomURL_Plugin.hpp"
#include "plugins/PropertyList.hpp"
#include "plugins/NeopixelVE.hpp"
//#include <ESP8266WiFiMulti.h>
#include "WiFiManager.h"
#ifdef VESP_PING_SSL
#include <WiFiClientSecure.h>
#endif

WiFiManager *wifiManager = NULL;
ESP8266WiFiMulti  *wifiMulti = NULL;
void stopAutoWifiConfig();

#define PROP_WIFI_STATIC_IP F("wifi.staticip")
#define PROP_WIFI_GATEWAY F("wifi.gateway")
#define PROP_WIFI_SUBNET F("wifi.subnet")
#define PROP_WIFI_DNS1 F("wifi.dns1")
#define PROP_WIFI_DNS2 F("wifi.dns2")

IPAddress ip = WiFi.localIP();
extern NeopixelVE neopixel; // there was a reason to put it here and not in commons
enum VSP_WIFI_STATE  {VSP_WIFI_CONNECTING, VSP_WIFI_CONNECTED, VSP_WIFI_NOCONFIG, VSP_WIFI_FAILED};
uint32_t lostWifiConnection = 0;

wl_status_t wifiState = WL_NO_SHIELD;

void handleWifi() {
  //if (wifiMulti && millis() > 8000) wifiMulti->run();
  //LOGGER << "ip = " << ip  << ", localip:" << WiFi.localIP() << endl;
  //if (wifiMulti) wifiMulti->run();
  if (wifiManager) wifiManager->loopConfigPortal();
  if (WiFi.status() == WL_NO_SSID_AVAIL && wifiManager == NULL && PowerManager.isWokeFromDeepSleep() == false) {
    startAutoWifiConfig("");
    //wifiState = WiFi.status();
    //Serial << "------State changed to: " << WiFi.status() << endl;
  }
  //delay(100);
  if (ip == WiFi.localIP()) return;
  else if (WiFi.status() == WL_CONNECTED) {
    ip = WiFi.localIP();
    #ifdef SAP_AUTH
        vSAP_Auth(EE_WIFI_P1_30B, EE_WIFI_P1_30B);
    #endif
    //neopixel.cmdLedHandleColorInst(F("ledcolor blue"));
    LOGGER << F("IP address: ") << WiFi.localIP() << F(" in ") << millis() << F(" ms") << endl << F("GOT IP") << endl;
    neopixel.signal(LED_WIFI_FOUND);
    stopAutoWifiConfig();
    if (!PowerManager.isWokeFromDeepSleep()) menuHandler.scheduleCommand("wss_start");
    //fireEvent("wifiConnected");

    // handleCommandVESPrino("vecmd led_mblue");
    // handleCommandVESPrino("vecmd ledmode_2_3");
  }
  ip = WiFi.localIP();
}

bool wifiAlreadyWaited = false;
wl_status_t waitForWifi(uint16_t timeoutMs) {
  if (wifiAlreadyWaited == true) return WiFi.status();
  if (WiFi.status() == WL_CONNECTED)  return WL_CONNECTED;
  //fireEvent("wifiSearching");
  LOGGER << F("\nWaiting for WiFi ");
  //bool putLF = false;
  int delayFix = 100;
  //const static uint32_t timeoutMs =1000L;
  bool a=true;
  uint32_t st=millis();
  for (int i=0; millis() < st + timeoutMs; i++) {
    if (WiFi.status() == WL_CONNECTED) break;
    delay(delayFix);
    handleWifi();
    menuHandler.loop();
    if ((i%10) == 0)  {
      LOGGER << '.';
      if (wifiAlreadyWaited) return WiFi.status();
      neopixel.signal(LED_WIFI_SEARCH);
    }
  }
  LOGGER << endl;
  if (wifiAlreadyWaited) return WiFi.status();
  if (WiFi.status() != WL_CONNECTED) neopixel.signal(LED_WIFI_FAILED);
  wifiAlreadyWaited = true;
  return WiFi.status();
}

void activeWait() {
  for (int i=1; i < 5; i++) {
    delay(100);
  //  handleWifi();
    menuHandler.loop();
    //menuHandler.processUserInput();
    if ((i%10) == 0) LOGGER << '.';
  }
  LOGGER << endl;
}


// //char x[30], y[30];
//
// int wifiConnectToStoredSSID() {
//   String ssid, pass;
//   ssid = PropertyList.readProperty(EE_WIFI_SSID);
//   pass = PropertyList.readProperty(EE_WIFI_P1);
//   LOGGER << F("Connecting to: \"") << ssid << F("\", \"") << pass << F("\"") << endl;
//   WiFi.disconnect();
//   delay(500);
//   WiFi.mode(WIFI_STA);
//   // strcpy(x, ssid.c_str());
//   // strcpy(y, pass.c_str());
//   // if (y[0] == 'B') {
//   // //  strcpy(x, "vladiHome");
//   //   strcpy(y, "0888414447");
//   // }
//   // LOGGER << F("Connecting to: \"") << x << F("\", \"") << y << F("\"") << endl;
//   WiFi.begin(ssid.c_str(), pass.c_str());
//   //WiFi.begin("MarinaResidence", "sdsa");
// }
void connectToWifi(const char *s1, const char *s2, const char *s3) {
  PropertyList.putProperty(EE_WIFI_SSID, s1);
  PropertyList.putProperty(EE_WIFI_P1, s2);
  PropertyList.putProperty(EE_WIFI_P2, s3);
  LOGGER << F("Connecting to: \"") << s1 << F("\", \"") << s2 << F("\"") << endl;
  WiFi.disconnect();
  delay(500);
  wifiConnectMulti();
  if (waitForWifi() != WL_CONNECTED) Serial << F("Failed to connect to WiFi") << endl;

  //wifiConnectToStoredSSID();
  //LOGGER << "Connecting to " << s1 << endl;
//  for (int i=0; i<10 && WiFi.status() != WL_CONNECTED; i--) {
//    handleWifi();
//    delay(1000);
//  }
//  handleWifi();
//  if (strstr(s1, "SAP-Guest")) {
//
//    checkSAPAuth();
//  }
}



void wifiScanNetworks(const char *ignore) {
  LOGGER.println(F("scan start"));
  WiFi.disconnect();
  delay(500);

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  LOGGER.println(F("scan done"));
  if (n == 0)
    LOGGER.println(F("no networks found"));
  else
  {
    LOGGER.print(n) ;
    LOGGER.println(F(" networks found"));
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      LOGGER.print(i + 1);
      LOGGER.print(F(": "));
      LOGGER.print(WiFi.SSID(i));
      LOGGER.print(F(" ("));
      LOGGER.print(WiFi.RSSI(i));
      LOGGER.print(F(")"));
      LOGGER.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?F(" "):F("*"));
      delay(10);
    }
  }
  LOGGER.println("");

  WiFi.mode(WIFI_STA);
  //wifiConnectToStoredSSID();

  // Wait a bit before scanning again

}

void setWifi(const char* p) {
  char s1[50], s2[130], s3[30];
  p = extractStringFromQuotes(p, s1, sizeof(s1));
  p = extractStringFromQuotes(p, s2, sizeof(s2));
  p = extractStringFromQuotes(p, s3, sizeof(s3));
  connectToWifi(s1, s2, s3);
}

void sendPingPort(const char *p) {
  char host[30],  port[20];
  bool secure = false;
  if (strstr(p, "pings")) secure = true;
  p = extractStringFromQuotes(p, host, 30);
  p = extractStringFromQuotes(p, port, 20);
  int iport = atoi(port);
  int res;
  LOGGER << F("Test connection to to:") << host << F(":") << port << F(", secure:") << secure << endl;

  if (secure) {
    #ifdef VESP_PING_SSL
      WiFiClientSecure ccc;
      res = ccc.connect(host, iport);
    #endif
  } else {
    WiFiClient ccc;
    res = ccc.connect(host, iport);
  }
  LOGGER << F("Res: ") << res << endl;


}

// void sndIOT(const char *line) {
//   if (WiFi.status() != WL_CONNECTED) {
//     LOGGER << F("Cannot send data. No Wifi connection.") << endl;
//     return;
//   }
// //  String path;
// //  path = PropertyList.hasProperty(EE_IOT_PATH);
//   if (PropertyList.hasProperty(EE_IOT_PATH)) {
//     SAP_HCP_IOT_Plugin::sndHCPIOT(line);
//   }
//
//
//   if (PropertyList.hasProperty(EE_GENIOT_PATH)) {
//     CustomURL_Plugin::sndGENIOT(line);
//   }
//
//   if (PropertyList.hasProperty(EE_MQTT_SERVER)) {
//     sendMQTT(&line[7]);
//   }
// }

void cmdSleeptype(const char *line) {
  int type = atoi(strchr(line, ' ') + 1);
  LOGGER << F("set sleep type to:") << type<< endl;
  switch (type) {
    case 0: WiFi.setSleepMode(WIFI_NONE_SLEEP);
    case 1: WiFi.setSleepMode(WIFI_MODEM_SLEEP);
    case 2: WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
  }
}

void cmdDelay(const char *line) {
  int d = atoi(strchr(line, ' ') + 1);
  LOGGER << F("delay for:") << d<< endl;
  delay(d);
}

void applyStaticWifiConfig() {
  if (PropertyList.hasProperty(PROP_WIFI_STATIC_IP)) {
    IPAddress ip, gw, su, d1, d2;
    ip.fromString(PropertyList.readProperty(PROP_WIFI_STATIC_IP));
    gw.fromString(PropertyList.readProperty(PROP_WIFI_GATEWAY));
    su.fromString(PropertyList.readProperty(PROP_WIFI_SUBNET));
    d1.fromString(PropertyList.readProperty(PROP_WIFI_DNS1));
    d2.fromString(PropertyList.readProperty(PROP_WIFI_DNS2));
    WiFi.config(ip, gw, su, d1, d2);
  }
}

void wifiConnectMulti() {
  if (wifiMulti) delete wifiMulti;
  //WiFi.forceSleepWake();
  //delay(100);
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  applyStaticWifiConfig();
  //wifiMulti = new ESP8266WiFiMulti();
  //wifiMulti->addAP("vladiHome", "0888414447");
  //wifiMulti->addAP("Andreev", "4506285842");
  String ssid = PropertyList.readProperty(EE_WIFI_SSID);
  String pass = PropertyList.readProperty(EE_WIFI_P1);
  if (ssid.length() && ssid.length() < 40 && pass.length() < 100) {
    //wifiMulti->addAP(ssid.c_str(), pass.c_str());
    char x[120], y[120];
    strcpy(x, ssid.c_str());
    strcpy(y, pass.c_str());
    //LOGGER << "wifibegin :: " << x << y << endl;
    wifiAlreadyWaited = false;
    WiFi.begin(x, y);
    neopixel.signal(LED_WIFI_SEARCH);
  } else {
    wifiAlreadyWaited = true;
    if (!PowerManager.isWokeFromDeepSleep()) menuHandler.scheduleCommand(F("autoconfig"));
  }
}

void wifiOff() {
  WiFi.disconnect(true); //mode = wifi Off
  // if (wifiMulti) {
  //   delete wifiMulti;
  //   wifiMulti = NULL;
  // }
  //WiFi.forceSleepBegin();
}


void setStaticWifi(const char* cmd) {
  cmd = strchr(cmd, ' ') + 1;
  char *x = strchr(cmd, ',');
  *x = 0;
  PropertyList.putProperty(PROP_WIFI_STATIC_IP, cmd);

  cmd = x+1;
  x = strchr(cmd, ',');
  *x = 0;
  PropertyList.putProperty(PROP_WIFI_GATEWAY, cmd);
  cmd = x+1;
  x = strchr(cmd, ',');
  *x = 0;
  PropertyList.putProperty(PROP_WIFI_SUBNET, cmd);
  cmd = x+1;
  x = strchr(cmd, ',');
  *x = 0;
  PropertyList.putProperty(PROP_WIFI_DNS1, cmd);
  cmd = x+1;
  x = strchr(cmd, ',');
  *x = 0;
  PropertyList.putProperty(PROP_WIFI_DNS2, cmd);
  //applyStaticWifiConfig();
}

void cmdIPConfig(const char *ignore) {
  LOGGER << F("IP: ") << WiFi.localIP() << endl;
  LOGGER << F("GW: ") << WiFi.gatewayIP() << endl;
  LOGGER << F("SU: ") << WiFi.subnetMask() << endl;
  LOGGER << F("D1: ") << WiFi.dnsIP(0) << endl;
  LOGGER << F("D2: ") << WiFi.dnsIP(1) << endl;
}

void WIFI_registerCommands(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("scan"), CMD_EXACT, &wifiScanNetworks, F("Scan available WiFi networks")));
  handler->registerCommand(new MenuEntry(F("wifi"), CMD_BEGIN, &setWifi, F("wifi \"SSID\", \"PASS\"")));
  handler->registerCommand(new MenuEntry(F("static_ip"), CMD_BEGIN, &setStaticWifi, F("static_ip ip,gateway,subnetMask,dns1,dns2")));
  handler->registerCommand(new MenuEntry(F("ping"), CMD_BEGIN, sendPingPort, F("")));
  //handler->registerCommand(new MenuEntry(F("sndiot"), CMD_BEGIN, sndIOT, F("")));
  handler->registerCommand(new MenuEntry(F("sleeptype"), CMD_BEGIN, cmdSleeptype, F("")));
  handler->registerCommand(new MenuEntry(F("delay"), CMD_BEGIN, cmdDelay, F("")));
  handler->registerCommand(new MenuEntry(F("ipconfig"), CMD_EXACT, cmdIPConfig, F("Dump IP configuration")));
  handler->registerCommand(new MenuEntry(F("autoconfig"), CMD_EXACT, startAutoWifiConfig, F("startAutoWifiConfig")));
}

void cbOnSaveConfigCallback() {
  PropertyList.putProperty(EE_WIFI_SSID, WiFi.SSID().c_str());
  PropertyList.putProperty(EE_WIFI_P1, WiFi.psk().c_str());
  stopAutoWifiConfig();
}

void stopAutoWifiConfig() {
  if (wifiManager) {
    wifiManager->stopConfigPortalAsync();
    delete wifiManager;
    wifiManager = NULL;
    menuHandler.scheduleCommand(F("ledcolor seqcncncncn"));
  }
}

extern NeopixelVE neopixel;
void startAutoWifiConfig(const char *ch) {
  // char custom_http1[140] = "Custom HTTP URL 1";
  // char custom_http2[140] = "Custom HTTP URL 2";
  // char custom_http3[140] = "Custom HTTP URL 3";
  // char mqtt_server[40] = "mqtt_server";
  // char mqtt_port[6] = "1883";
  // char mqtt_msg1[140] = "<mqtt topic>:<msg1>";
  // char mqtt_msg2[140] = "<mqtt topic>:<msg2>";
  // char mqtt_msg3[140] = "<mqtt topic>:<msg3>";
  shouldSend = false;
  // WiFiManagerParameter par_custom_http1("http1", "http1", custom_http1, 140);
  // WiFiManagerParameter par_custom_http2("http2", "http2", custom_http2, 140);
  // WiFiManagerParameter par_custom_http3("http3", "http3", custom_http3, 140);
  // WiFiManagerParameter par_custom_mqtt_server("mqttserver", "mqtt server", mqtt_server, 40);
  // WiFiManagerParameter par_custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  // WiFiManagerParameter par_custom_mqtt_msg1("mqmsg1", "mqmsg1", mqtt_msg1, 140);
  // WiFiManagerParameter par_custom_mqtt_msg2("mqmsg2", "mqmsg2", mqtt_msg2, 140);
  // WiFiManagerParameter par_custom_mqtt_msg3("mqmsg3", "mqmsg3", mqtt_msg3, 140);
  // wifiManager.addParameter(&par_custom_http1);
  // wifiManager.addParameter(&par_custom_http2);
  // wifiManager.addParameter(&par_custom_http3);
  // wifiManager.addParameter(&par_custom_mqtt_server);
  // wifiManager.addParameter(&par_custom_mqtt_port);
  // wifiManager.addParameter(&par_custom_mqtt_msg1);
  // wifiManager.addParameter(&par_custom_mqtt_msg2);
  // wifiManager.addParameter(&par_custom_mqtt_msg3);
  neopixel.cmdLedSetBrgInst(F("ledbrg 80"));
  neopixel.cmdLedHandleColorInst(F("ledcolor mblue"));

  WiFi.persistent(false);
  wifiManager = new WiFiManager();
  //wifiManager.setConnectTimeout(10);
  String chipId = String(ESP.getChipId(), HEX);
  chipId.toUpperCase();

  String name = String(F("vAirMonitor_")) + chipId;
  //wifiManager.autoConnect(name.c_str());
  wifiManager->setSaveConfigCallback(cbOnSaveConfigCallback);
  wifiManager->startConfigPortalAsync(name.c_str());

  // menuHandler.handleCommand(F("custom_url_clean"));
  // if (par_custom_http1.getValue()[0]) menuHandler.handleCommand((String(F("custom_url_add \"0\",\"")) + par_custom_http1.getValue() + "\"").c_str());
  // if (par_custom_http2.getValue()[0]) menuHandler.handleCommand((String(F("custom_url_add \"1\",\"")) + par_custom_http2.getValue() + "\"").c_str());
  // if (par_custom_http3.getValue()[0]) menuHandler.handleCommand((String(F("custom_url_add \"2\",\"")) + par_custom_http3.getValue() + "\"").c_str());
  // if (par_custom_mqtt_server.getValue()[0]) {
  //   char x[200];
  //   sprintf(x, String(F("mqtt_setup \"%s\",\"%s\",\"\",\"\",\"\"")).c_str(), par_custom_mqtt_server.getValue(), par_custom_mqtt_port.getValue());
  //   menuHandler.handleCommand(x);
  // }
  // menuHandler.handleCommand(F("mqtt_msg_clean"));
  // if (par_custom_mqtt_msg1.getValue()[0]) menuHandler.handleCommand((String(F("mqtt_msg_add \"0\"")) + par_custom_mqtt_msg1.getValue()).c_str());
  // if (par_custom_mqtt_msg2.getValue()[0]) menuHandler.handleCommand((String(F("mqtt_msg_add \"1\"")) + par_custom_mqtt_msg2.getValue()).c_str());
  // if (par_custom_mqtt_msg3.getValue()[0]) menuHandler.handleCommand((String(F("mqtt_msg_add \"2\"")) + par_custom_mqtt_msg3.getValue()).c_str());
  //
//  PropertyList.putProperty(EE_WIFI_SSID, WiFi.SSID().c_str());
//  PropertyList.putProperty(EE_WIFI_P1, WiFi.psk().c_str());
//  menuHandler.scheduleCommand(F("restart"));
  // menuHandler.handleCommand(F("prop_list"));
  // neopixel.cmdLedSetBrgInst(F("ledbrg 99"));
  // neopixel.cmdLedHandleColorInst(F("ledcolor lila"));
  //LOGGER << "wifiaaaaaaaaa: " << WiFi.SSID() << " : " << WiFi.psk() << endl;
}
