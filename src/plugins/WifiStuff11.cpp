#include "plugins/WifiStuff.hpp"
#define PROP_WIFI_STATIC_IP F("wifi.staticip")
#define PROP_WIFI_GATEWAY F("wifi.gateway")
#define PROP_WIFI_SUBNET F("wifi.subnet")
#define PROP_WIFI_DNS1 F("wifi.dns1")
#define PROP_WIFI_DNS2 F("wifi.dns2")

//enum VSP_WIFI_STATE  {VSP_WIFI_CONNECTING, VSP_WIFI_CONNECTED, VSP_WIFI_NOCONFIG, VSP_WIFI_FAILED};
extern WifiStuffClass WifiStuff;
extern NeopixelVE neopixel; // there was a reason to put it here and not in commons
void registerPlugin(Plugin *plugin);
WifiStuffClass::WifiStuffClass() {
  registerPlugin(this);
}

void WifiStuffClass::onProperty(String &key, String &value) {
  if (key == PROP_WIFI_STATIC_IP) staticIp.fromString(value);
  else if (key == PROP_WIFI_GATEWAY) gateway.fromString(value);
  else if (key == PROP_WIFI_SUBNET) subnet.fromString(value);
  else if (key == PROP_WIFI_DNS1) dns1.fromString(value);
  else if (key == PROP_WIFI_DNS2) dns2.fromString(value);
  else if (key == EE_WIFI_SSID) ssid = value;
  else if (key == EE_WIFI_P1) pass = value;
  //LOGGER << "onProp:" << key << ":" << value << endl;
}
void WifiStuffClass::handleWifi() {
  //if (wifiMulti && millis() > 8000) wifiMulti->run();
  //LOGGER << "ip = " << ip  << ", localip:" << WiFi.localIP() << endl;
  //if (wifiMulti) wifiMulti->run();
  //LOGGER << "Wifi state hw: " << WiFi.status()<< endl;

  if (wifiManager) wifiManager->loopConfigPortal();
  if (WiFi.status() == WL_NO_SSID_AVAIL && wifiManager == NULL && PowerManager.isWokeFromDeepSleep() == false) {
    startAutoWifiConfig("");
    //wifiState = WiFi.status();
    //Serial << "------State changed to: " << WiFi.status() << endl;
  }
  //delay(100);
  if (ip == WiFi.localIP() && WiFi.status() == WL_CONNECTED && wifiState == WL_CONNECTED) return;
  else if (WiFi.status() == WL_CONNECTED) {
    ip = WiFi.localIP();
    #ifdef SAP_AUTH
        vSAP_Auth(EE_WIFI_P1_30B, EE_WIFI_P1_30B);
    #endif
    //neopixel.cmdLedHandleColorInst(F("ledcolor blue"));
    LOGGER << F("IP address: ") << WiFi.localIP() << F(" in ") << millis() << F(" ms") << endl << F("GOT IP") << endl;
    neopixel.signal(LED_WIFI_FOUND);
    stopAutoWifiConfig();
    if (!PowerManager.isWokeFromDeepSleep()) {
      menuHandler.scheduleCommand("wss_start");
      storeStaticWifiInRTC();
    }

    //fireEvent("wifiConnected");

    // handleCommandVESPrino("vecmd led_mblue");
    // handleCommandVESPrino("vecmd ledmode_2_3");
  }
  ip = WiFi.localIP();
  wifiState = WiFi.status();
}


wl_status_t WifiStuffClass::waitForWifi(uint16_t timeoutMs) {
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

void WifiStuffClass::activeWait() {
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
void WifiStuffClass::connectToWifi(const char *s1, const char *s2, const char *s3) {
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



void WifiStuffClass::wifiScanNetworks(const char *ignore) {
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

void WifiStuffClass::setWifi(const char* p) {
  char s1[50], s2[130], s3[30];
  p = extractStringFromQuotes(p, s1, sizeof(s1));
  p = extractStringFromQuotes(p, s2, sizeof(s2));
  p = extractStringFromQuotes(p, s3, sizeof(s3));
  WifiStuff.connectToWifi(s1, s2, s3);
}

void WifiStuffClass::sendPingPort(const char *p) {
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

void WifiStuffClass::cmdSleeptype(const char *line) {
  int type = atoi(strchr(line, ' ') + 1);
  LOGGER << F("set sleep type to:") << type<< endl;
  switch (type) {
    case 0: WiFi.setSleepMode(WIFI_NONE_SLEEP);
    case 1: WiFi.setSleepMode(WIFI_MODEM_SLEEP);
    case 2: WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
  }
}

void WifiStuffClass::cmdDelay(const char *line) {
  int d = atoi(strchr(line, ' ') + 1);
  LOGGER << F("delay for:") << d<< endl;
  delay(d);
}

void WifiStuffClass::applyStaticWifiConfig() {
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

void WifiStuffClass::loadStaticIPConfigFromRTC() {
  staticIp = rtcMemStore.getGenData(GEN_WIFI_STATIC_IP);
  gateway = rtcMemStore.getGenData(GEN_WIFI_GW);
  subnet = rtcMemStore.getGenData(GEN_WIFI_SUB);
  dns1 = rtcMemStore.getGenData(GEN_WIFI_DNS1);
  dns2 = rtcMemStore.getGenData(GEN_WIFI_DNS2);
}

void WifiStuffClass::storeStaticWifiInRTC() {
  rtcMemStore.setGenData(GEN_WIFI_STATIC_IP, WiFi.localIP());
  rtcMemStore.setGenData(GEN_WIFI_GW, WiFi.gatewayIP());
  rtcMemStore.setGenData(GEN_WIFI_SUB, WiFi.subnetMask());
  rtcMemStore.setGenData(GEN_WIFI_DNS1, WiFi.dnsIP(0));
  rtcMemStore.setGenData(GEN_WIFI_DNS2, WiFi.dnsIP(1));
}

void WifiStuffClass::wifiConnectMulti() {
  //if (wifiMulti) delete wifiMulti;
  //WiFi.forceSleepWake();
  //delay(100);
  PERF("WIFI 1")
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  //LOGGER << "Wifi state 1: " << WiFi.status()<< endl;
  //delay(500);
  //LOGGER << "Wifi state 2: " << WiFi.status()<< endl;
  WiFi.mode(WIFI_STA);
  PERF("WIFI 2")
  if (PowerManager.isWokeFromDeepSleep()) {
    loadStaticIPConfigFromRTC();
  }
  if (staticIp != 0) {
    WiFi.config(staticIp, gateway, subnet, dns1, dns2);
  }
  //applyStaticWifiConfig();
  PERF("WIFI 3")

  //wifiMulti = new ESP8266WiFiMulti();
  //wifiMulti->addAP("vladiHome", "0888414447");
  //wifiMulti->addAP("Andreev", "4506285842");
  // String ssid = PropertyList.readProperty(EE_WIFI_SSID);
  // String pass = PropertyList.readProperty(EE_WIFI_P1);
  PERF("WIFI 4")

  if (ssid.length() && ssid.length() < 40 && pass.length() < 100) {
    //wifiMulti->addAP(ssid.c_str(), pass.c_str());
    char x[120], y[120];
    strcpy(x, ssid.c_str());
    strcpy(y, pass.c_str());
    //LOGGER << "wifibegin :: " << x << y << endl;
    wifiAlreadyWaited = false;
    //LOGGER << "Wifi state 3: " << WiFi.status()<< endl;

    WiFi.begin(x, y);
    neopixel.signal(LED_WIFI_SEARCH);
    PERF("WIFI 5")
    //LOGGER << "Wifi state 4: " << WiFi.status()<< endl;


  } else {
    wifiAlreadyWaited = true;
    if (!PowerManager.isWokeFromDeepSleep()) menuHandler.scheduleCommand(F("autoconfig"));
  }
}

void WifiStuffClass::wifiOff() {
  WiFi.disconnect(true); //mode = wifi Off
  // if (wifiMulti) {
  //   delete wifiMulti;
  //   wifiMulti = NULL;
  // }
  //WiFi.forceSleepBegin();
}


void WifiStuffClass::setStaticWifi(const char* cmd) {
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

void WifiStuffClass::cmdIPConfig(const char *ignore) {
  LOGGER << F("IP: ") << WiFi.localIP() << endl;
  LOGGER << F("GW: ") << WiFi.gatewayIP() << endl;
  LOGGER << F("SU: ") << WiFi.subnetMask() << endl;
  LOGGER << F("D1: ") << WiFi.dnsIP(0) << endl;
  LOGGER << F("D2: ") << WiFi.dnsIP(1) << endl;
}

bool WifiStuffClass::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("scan"), CMD_EXACT, WifiStuffClass::wifiScanNetworks, F("Scan available WiFi networks")));
  handler->registerCommand(new MenuEntry(F("wifi"), CMD_BEGIN, WifiStuffClass::setWifi, F("wifi \"SSID\", \"PASS\"")));
  handler->registerCommand(new MenuEntry(F("static_ip"), CMD_BEGIN, WifiStuffClass::setStaticWifi, F("static_ip ip,gateway,subnetMask,dns1,dns2")));
  handler->registerCommand(new MenuEntry(F("ping"), CMD_BEGIN, WifiStuffClass::sendPingPort, F("")));
  //handler->registerCommand(new MenuEntry(F("sndiot"), CMD_BEGIN, sndIOT, F("")));
  handler->registerCommand(new MenuEntry(F("sleeptype"), CMD_BEGIN, WifiStuffClass::cmdSleeptype, F("")));
  handler->registerCommand(new MenuEntry(F("delay"), CMD_BEGIN, WifiStuffClass::cmdDelay, F("")));
  handler->registerCommand(new MenuEntry(F("ipconfig"), CMD_EXACT, WifiStuffClass::cmdIPConfig, F("Dump IP configuration")));
  handler->registerCommand(new MenuEntry(F("autoconfig"), CMD_EXACT, WifiStuffClass::startAutoWifiConfig, F("startAutoWifiConfig")));
  return false;
}

void WifiStuffClass::cbOnSaveConfigCallback() {
  PropertyList.putProperty(EE_WIFI_SSID, WiFi.SSID().c_str());
  PropertyList.putProperty(EE_WIFI_P1, WiFi.psk().c_str());
  WifiStuff.stopAutoWifiConfig();
}

void WifiStuffClass::stopAutoWifiConfig() {
  if (wifiManager) {
    wifiManager->stopConfigPortalAsync();
    delete wifiManager;
    wifiManager = NULL;
    menuHandler.scheduleCommand(F("ledcolor seqcncncncn"));
  }
}

void WifiStuffClass::startAutoWifiConfig(const char *ch) {
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
  WifiStuff.wifiManager = new WiFiManager();
  //wifiManager.setConnectTimeout(10);
  String chipId = String(ESP.getChipId(), HEX);
  chipId.toUpperCase();

  String name = String(F("vAirMonitor_")) + chipId;
  //wifiManager.autoConnect(name.c_str());
  WifiStuff.wifiManager->setSaveConfigCallback(WifiStuffClass::cbOnSaveConfigCallback);
  WifiStuff.wifiManager->startConfigPortalAsync(name.c_str());

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
