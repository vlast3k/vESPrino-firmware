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
    #ifndef HARDCODED_SENSORS
    startAutoWifiConfig("");
    #endif
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
    neopixel.signal(LED_WIFI_FOUND, SIGNAL_FIRST);
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
    menuHandler.loop();
    handleWifi();
    if ((i%10) == 0)  {
      LOGGER << '.';
      if (wifiAlreadyWaited) return WiFi.status();
      neopixel.signal(LED_WIFI_SEARCH);
    }
  }
  LOGGER << endl;
  if (wifiAlreadyWaited) return WiFi.status();
  if (WiFi.status() != WL_CONNECTED) neopixel.signal(LED_WIFI_FAILED, SIGNAL_FIRST);
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

void WifiStuffClass::connectToWifi(const char *s1, const char *s2, const char *s3) {
  PropertyList.putProperty(EE_WIFI_SSID, s1);
  PropertyList.putProperty(EE_WIFI_P1, s2);
  PropertyList.putProperty(EE_WIFI_P2, s3);
  LOGGER << F("Connecting to: \"") << s1 << F("\", \"") << s2 << F("\"") << endl;
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
  delay(2500);
  wifiConnectMulti();
  if (waitForWifi() != WL_CONNECTED) Serial << F("Failed to connect to WiFi") << endl;
}



void WifiStuffClass::wifiScanNetworks(const char *ignore) {
  LOGGER.println(F("scan start"));
  WiFi.disconnect();
  delay(500);

  WiFi.mode(WIFI_STA);

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
    //LOGGER << "Setting up static ip: " << staticIp << gateway << subnet << dns1 << dns2 << endl;
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
    #ifndef HARDCODED_SENSORS
    if (!PowerManager.isWokeFromDeepSleep()) menuHandler.scheduleCommand(F("autoconfig"));
    #endif
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
    menuHandler.scheduleCommand(F("ledcolor seqcgcgcgn"));
    menuHandler.scheduleCommand(F("restart"));
  }
}

void WifiStuffClass::startAutoWifiConfig(const char *ch) {
  shouldSend = false;
  neopixel.cmdLedHandleColorInst(F("ledcolor seq80m"));

  WiFi.persistent(false);
  WifiStuff.wifiManager = new WiFiManager();
  //wifiManager.setConnectTimeout(10);
  String chipId = String(ESP.getChipId(), HEX);
  chipId.toUpperCase();

  String name = String(F("vAirMonitor_")) + chipId;
  //wifiManager.autoConnect(name.c_str());
  WifiStuff.wifiManager->setConnectTimeout(14);
  WifiStuff.wifiManager->setSaveConfigCallback(WifiStuffClass::cbOnSaveConfigCallback);
  WifiStuff.wifiManager->startConfigPortalAsync(name.c_str());
}
