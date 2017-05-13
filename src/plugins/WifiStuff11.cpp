#include "plugins/WifiStuff.hpp"
#define PROP_WIFI_STATIC_IP F("wifi.staticip")
#define PROP_WIFI_GATEWAY F("wifi.gateway")
#define PROP_WIFI_SUBNET F("wifi.subnet")
#define PROP_WIFI_DNS1 F("wifi.dns1")
#define PROP_WIFI_DNS2 F("wifi.dns2")
#define PROP_WSSERVER_DISABLE F("wss.disable")
#define PROP_AUTOCFG_DISABLE F("autocfg.disable")
#define PROP_DISABLE_IP_REUSE F("wifi.disable.ipreuse")

//enum VSP_WIFI_STATE  {VSP_WIFI_CONNECTING, VSP_WIFI_CONNECTED, VSP_WIFI_NOCONFIG, VSP_WIFI_FAILED};
extern WifiStuffClass WifiStuff;
extern NeopixelVE neopixel; // there was a reason to put it here and not in commons
void registerPlugin(Plugin *plugin);
WifiStuffClass::WifiStuffClass() {
  registerPlugin(this);
}

void WifiStuffClass::onProperty(String &key, String &value) {
  if (key == PROP_WIFI_STATIC_IP)    staticIp.fromString(value);
  else if (key == PROP_WIFI_GATEWAY) gateway.fromString(value);
  else if (key == PROP_WIFI_SUBNET)  subnet.fromString(value);
  else if (key == PROP_WIFI_DNS1)    dns1.fromString(value);
  else if (key == PROP_WIFI_DNS2)    dns2.fromString(value);
  else if (key == EE_WIFI_SSID)      ssid = value;
  else if (key == EE_WIFI_P1)        pass = value;
  else if (key == PROP_WSSERVER_DISABLE) wssDisable = PropertyList.toBool(value);
  else if (key == PROP_AUTOCFG_DISABLE)  autoCfgDisable = PropertyList.toBool(value);
  else if (key == PROP_DISABLE_IP_REUSE)  ipReuse = !PropertyList.toBool(value);
}

void WifiStuffClass::handleWifi() {
  //if (wifiMulti && millis() > 8000) wifiMulti->run();
  //LOGGER << "ip = " << ip  << ", localip:" << WiFi.localIP() << endl;
  //if (wifiMulti) wifiMulti->run();
  //LOGGER << "Wifi state hw: " << WiFi.status()<< endl;

  if (wifiManager) wifiManager->loopConfigPortal();
  if (!SLAVE && WiFi.status() != WL_CONNECTED
       && wifiAlreadyWaited
       && wifiManager == NULL
       && PowerManager.isWokeFromDeepSleep() == false
       && !autoCfgDisable
     ) {
    #ifndef HARDCODED_SENSORS
    //Serial << "ASDSADSADSAD" << endl;
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
    #ifndef HARDCODED_SENSORS
    neopixel.signal(LED_WIFI_FOUND, SIGNAL_FIRST);
    #endif
    stopAutoWifiConfig();
    if (!SLAVE && !PowerManager.isWokeFromDeepSleep() && !wssDisable) {
      menuHandler.scheduleCommand("wss_start");

    }
    storeStaticWifiInRTC();
    //wifiAlreadyWaited = millis();
    autoCfgDisable = true;

    //fireEvent("wifiConnected");

    // handleCommandVESPrino("vecmd led_mblue");
    // handleCommandVESPrino("vecmd ledmode_2_3");
  }
  ip = WiFi.localIP();
  wifiState = WiFi.status();
}


wl_status_t WifiStuffClass::waitForWifi(String from, uint16_t timeoutMs) {
  if (WiFi.status() == WL_CONNECTED)  return WL_CONNECTED;
  if (wifiAlreadyWaited && millis() - wifiAlreadyWaited < 60000L) return WiFi.status();
  if (wifiAlreadyWaited) wifiConnectMulti();
  //WiFi.setOutputPower(30);
  //fireEvent("wifiSearching");
  LOGGER << F("\nWaiting for WiFi from: ") << from;
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
    if (WiFi.status() == WL_NO_SSID_AVAIL) {
      Serial << F("SSID [") << ssid << F("] not found") << endl;
      break;
    }
    if ((i%10) == 0)  {
      LOGGER << '.';
      if (wifiAlreadyWaited && millis() - wifiAlreadyWaited < 60000L) return WiFi.status();
      //Serial << "------State changed to: " << WiFi.status() << endl;
      //neopixel.signal(LED_WIFI_SEARCH);
    }
  }
  //Serial << "------State changed to: " << WiFi.status() << endl;
  LOGGER << endl;
  if (wifiAlreadyWaited && millis() - wifiAlreadyWaited < 60000L) return WiFi.status();
  #ifndef HARDCODED_SENSORS
  if (WiFi.status() != WL_CONNECTED) neopixel.signal(LED_WIFI_FAILED, SIGNAL_FIRST);
  #endif
  wifiAlreadyWaited = millis();
  handleFailedConnect();
  return WiFi.status();
}

void WifiStuffClass::handleFailedConnect() {
  if (WiFi.status() == WL_CONNECTED) return;
  int failedConnects = rtcMemStore.getGenData(GEN_WIFI_FAILED_CONN);
  if (failedConnects > 5) {
    PowerManager.quietRestart();
  } else {
    rtcMemStore.setGenData(GEN_WIFI_FAILED_CONN, 1 + failedConnects);
  }
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
  if (waitForWifi(F("Connect")) != WL_CONNECTED) Serial << F("Failed to connect to WiFi") << endl;
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

void WifiStuffClass::clearStaticIPConfigFromRTC() {
  rtcMemStore.setGenData(GEN_WIFI_STATIC_IP, 0);
}

void WifiStuffClass::loadStaticIPConfigFromRTC() {
  if (rtcMemStore.getGenData(GEN_WIFI_STATIC_IP) == 0) return;
  staticIp = rtcMemStore.getGenData(GEN_WIFI_STATIC_IP);
  gateway = rtcMemStore.getGenData(GEN_WIFI_GW);
  subnet = rtcMemStore.getGenData(GEN_WIFI_SUB);
  dns1 = rtcMemStore.getGenData(GEN_WIFI_DNS1);
  dns2 = rtcMemStore.getGenData(GEN_WIFI_DNS2);
}

void WifiStuffClass::storeStaticWifiInRTC() {
  if (WiFi.localIP() == (uint32_t)0) return;
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
  if (PowerManager.isWokeFromDeepSleep() && ipReuse) {
    loadStaticIPConfigFromRTC();
    clearStaticIPConfigFromRTC(); // in case connection fails, next time it will take new ip
    LOGGER << "USING IP From RTC:" << staticIp.toString() << endl;
  }
  applyStaticWifiConfig();

  #ifdef HARDCODED_SENSORS
  if (ssid.length() == 0) {
    ssid = "vladiHome";
    pass = "0888414447";
  }
  #endif

  if (ssid.length() && ssid.length() < 40 && pass.length() < 100) {
    //wifiMulti->addAP(ssid.c_str(), pass.c_str());
    char x[120], y[120];
    strcpy(x, ssid.c_str());
    strcpy(y, pass.c_str());
    wifiAlreadyWaited = 0;
    WiFi.persistent(false);
    WiFi.disconnect();
    delay(100);
    WiFi.mode(WIFI_OFF);
    delay(100);
    WiFi.mode(WIFI_STA);

    if (staticIp != 0) {
      //LOGGER << "Setting up static ip: " << staticIp << gateway << subnet << dns1 << dns2 << endl;
      WiFi.config(staticIp, gateway, subnet, dns1, dns2);
    }

    WiFi.begin(x, y);
    PERF("WIFI 5")
  } else {
    wifiAlreadyWaited = millis();
    #ifndef HARDCODED_SENSORS
    if (!SLAVE && !PowerManager.isWokeFromDeepSleep() && !autoCfgDisable) menuHandler.scheduleCommand(F("autoconfig"));
    #endif
  }
}

void WifiStuffClass::wifiOff() {
  WiFi.disconnect(true); //mode = wifi Off
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
  WifiStuff.cbOnSaveConfigCallbackInst();
  // PropertyList.putProperty(EE_WIFI_SSID, (wifiManager->_ssid).c_str());
  // PropertyList.putProperty(EE_WIFI_P1, (wifiManager->_pass).c_str());
  // WifiStuff.stopAutoWifiConfig();
}

void WifiStuffClass::cbOnSaveConfigCallbackInst() {
  PropertyList.putProperty(EE_WIFI_SSID, (wifiManager->_ssid).c_str());
  PropertyList.putProperty(EE_WIFI_P1, (wifiManager->_pass).c_str());
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
  if (WifiStuff.wifiManager) return;
  shouldSend = false;
  neopixel.cmdLedHandleColorInst(F("ledcolor seq1m"));
  WifiStuff.colorAfterWifiAutoConfig = neopixel.getCurrentColor();

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

void WifiStuffClass::loop() {
  if (wifiManager && millis() > 5L * 60 *1000) {
    Serial << F("Stopped WiFi Autoconfig");
    wifiManager->stopConfigPortalAsync();
    delete wifiManager;
    wifiManager = NULL;
    autoCfgDisable = true;
    if (colorAfterWifiAutoConfig == neopixel.getCurrentColor()) {
      neopixel.cmdLedHandleColorInst(F("ledcolor black"));
    }
  }
}

void WifiStuffClass::noWifi() {
  wifiAlreadyWaited = 0;
  WiFi.disconnect();
  WiFi.mode(WIFI_OFF);
}
