#include "common.hpp"
#include "plugins\SAP_HCP_IOT_Plugin.hpp"
#include "plugins\CustomURL_Plugin.hpp"
#include "plugins\PropertyList.hpp"
#include <ESP8266WiFiMulti.h>

ESP8266WiFiMulti  *wifiMulti = NULL;

IPAddress ip = WiFi.localIP();

void handleWifi() {
  if (millis() > 8000) wifiMulti->run();
  //SERIAL_PORT << "ip = " << ip  << ", localip:" << WiFi.localIP() << endl;
  if (ip == WiFi.localIP()) return;
  else if (WiFi.status() == WL_CONNECTED) {
    ip = WiFi.localIP();
#ifdef SAP_AUTH
    vSAP_Auth(EE_WIFI_P1_30B, EE_WIFI_P1_30B);
#endif
    SERIAL_PORT << F("IP address: ") << WiFi.localIP() << F(" in ") << millis() << F(" ms") << endl << F("GOT IP") << endl;
    // handleCommandVESPrino("vecmd led_mblue");
    // handleCommandVESPrino("vecmd ledmode_2_3");
  }
  ip = WiFi.localIP();
}

wl_status_t waitForWifi(uint16_t timeoutMs) {
  bool putLF = false;
  int delayFix = 100;
  //const static uint32_t timeoutMs =1000L;
  for (int i=0; i*delayFix < timeoutMs; i++) {
    if (WiFi.status() == WL_CONNECTED) break;
    delay(delayFix);
    handleWifi();
    menuHandler.loop();
    if ((i%10) == 0) {
      SERIAL_PORT << '.';
      putLF = true;
    }
  }
  return WiFi.status();
}

void activeWait() {
  for (int i=1; i < 31; i++) {
    delay(100);
  //  handleWifi();
    menuHandler.loop();
    //menuHandler.processUserInput();
    if ((i%10) == 0) SERIAL_PORT << '.';
  }
  SERIAL_PORT << endl;
}


//char x[30], y[30];

int wifiConnectToStoredSSID() {
  String ssid, pass;
  ssid = PropertyList.readProperty(EE_WIFI_SSID);
  pass = PropertyList.readProperty(EE_WIFI_P1);
  SERIAL_PORT << F("Connecting to: \"") << ssid << F("\", \"") << pass << F("\"") << endl;
  WiFi.disconnect();
  delay(500);
  WiFi.mode(WIFI_STA);
  // strcpy(x, ssid.c_str());
  // strcpy(y, pass.c_str());
  // if (y[0] == 'B') {
  // //  strcpy(x, "vladiHome");
  //   strcpy(y, "0888414447");
  // }
  // SERIAL_PORT << F("Connecting to: \"") << x << F("\", \"") << y << F("\"") << endl;
  WiFi.begin(ssid.c_str(), pass.c_str());
  //WiFi.begin("MarinaResidence", "sdsa");
}
void connectToWifi(const char *s1, const char *s2, const char *s3) {
  PropertyList.putProperty(EE_WIFI_SSID, s1);
  PropertyList.putProperty(EE_WIFI_P1, s2);
  PropertyList.putProperty(EE_WIFI_P2, s3);
  SERIAL_PORT << F("Connecting to: \"") << s1 << F("\", \"") << s2 << F("\"") << endl;
  wifiConnectMulti();
  //wifiConnectToStoredSSID();
  //SERIAL_PORT << "Connecting to " << s1 << endl;
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
  SERIAL_PORT.println(F("scan start"));
  WiFi.disconnect();
  delay(500);

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  SERIAL_PORT.println(F("scan done"));
  if (n == 0)
    SERIAL_PORT.println(F("no networks found"));
  else
  {
    SERIAL_PORT.print(n) ;
    SERIAL_PORT.println(F(" networks found"));
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      SERIAL_PORT.print(i + 1);
      SERIAL_PORT.print(F(": "));
      SERIAL_PORT.print(WiFi.SSID(i));
      SERIAL_PORT.print(F(" ("));
      SERIAL_PORT.print(WiFi.RSSI(i));
      SERIAL_PORT.print(F(")"));
      SERIAL_PORT.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?F(" "):F("*"));
      delay(10);
    }
  }
  SERIAL_PORT.println("");

  WiFi.mode(WIFI_STA);
  wifiConnectToStoredSSID();

  // Wait a bit before scanning again

}

void setWifi(const char* p) {
  char s1[30], s2[30], s3[30];
  p = extractStringFromQuotes(p, s1, 29);
  p = extractStringFromQuotes(p, s2, 29);
  p = extractStringFromQuotes(p, s3, 29);
  connectToWifi(s1, s2, s3);
}

void sendPingPort(const char *p) {
  char host[30],  port[20];
  p = extractStringFromQuotes(p, host, 30);
  p = extractStringFromQuotes(p, port, 20);
  int iport = atoi(port);
  WiFiClient ccc;
  SERIAL_PORT << "Test connection to to:" << host << ":" << port << endl;
  int res = ccc.connect(host, iport);
  SERIAL_PORT << "Res: " << res << endl;

}

void sndIOT(const char *line) {
  if (WiFi.status() != WL_CONNECTED) {
    SERIAL_PORT << F("Cannot send data. No Wifi connection.") << endl;
    return;
  }
//  String path;
//  path = PropertyList.hasProperty(EE_IOT_PATH);
  if (PropertyList.hasProperty(EE_IOT_PATH)) {
    SAP_HCP_IOT_Plugin::sndHCPIOT(line);
  }


  if (PropertyList.hasProperty(EE_GENIOT_PATH)) {
    CustomURL_Plugin::sndGENIOT(line);
  }

  if (PropertyList.hasProperty(EE_MQTT_SERVER)) {
    sendMQTT(&line[7]);
  }
}

void cmdSleeptype(const char *line) {
  int type = atoi(strchr(line, ' ') + 1);
  Serial << "set sleep type to:" << type<< endl;
  switch (type) {
    case 0: WiFi.setSleepMode(WIFI_NONE_SLEEP);
    case 1: WiFi.setSleepMode(WIFI_MODEM_SLEEP);
    case 2: WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
  }
}

void cmdDelay(const char *line) {
  int d = atoi(strchr(line, ' ') + 1);
  Serial << "delay for:" << d<< endl;
  delay(d);
}

void wifiConnectMulti() {
  if (wifiMulti) delete wifiMulti;
  wifiMulti = new ESP8266WiFiMulti();
  wifiMulti->addAP("vladiHome", "0888414447");
  wifiMulti->addAP("Andreev", "4506285842");
  String ssid = PropertyList.readProperty(EE_WIFI_SSID);
  String pass = PropertyList.readProperty(EE_WIFI_P1);
  WiFi.persistent(false);
  if (ssid.length() && ssid.length() < 40 && pass.length() < 100) {
    wifiMulti->addAP(ssid.c_str(), pass.c_str());
    char x[20], y[20];
    strcpy(x, ssid.c_str());
    strcpy(y, pass.c_str());
    WiFi.begin(x, y);
  }
}

void WIFI_registerCommands(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("scan"), CMD_EXACT, &wifiScanNetworks, F("")));
  handler->registerCommand(new MenuEntry(F("wifi"), CMD_BEGIN, &setWifi, F("")));
  handler->registerCommand(new MenuEntry(F("ping"), CMD_BEGIN, sendPingPort, F("")));
  handler->registerCommand(new MenuEntry(F("sndiot"), CMD_BEGIN, sndIOT, F("")));
  handler->registerCommand(new MenuEntry(F("sleeptype"), CMD_BEGIN, cmdSleeptype, F("")));
  handler->registerCommand(new MenuEntry(F("delay"), CMD_BEGIN, cmdDelay, F("")));
}
