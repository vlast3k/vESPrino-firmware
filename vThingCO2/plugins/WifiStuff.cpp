#include "common.hpp"
#include "plugins\SAP_HCP_IOT_Plugin.hpp"
#include "plugins\CustomURL_Plugin.hpp"
#include "plugins\PropertyList.hpp"
#include <ESP8266WiFiMulti.h>

ESP8266WiFiMulti  *wifiMulti = NULL;

IPAddress ip = WiFi.localIP();

void handleWifi() {
  if (wifiMulti) wifiMulti->run();
  //SERIAL << "ip = " << ip  << ", localip:" << WiFi.localIP() << endl;
  if (ip == WiFi.localIP()) return;
  else if (WiFi.status() == WL_CONNECTED) {
    ip = WiFi.localIP();
#ifdef SAP_AUTH
    vSAP_Auth(EE_WIFI_P1_30B, EE_WIFI_P1_30B);
#endif
    SERIAL << F("IP address: ") << WiFi.localIP() << F(" in ") << millis() << F(" ms") << endl << F("GOT IP") << endl;
    // handleCommandVESPrino("vecmd led_mblue");
    // handleCommandVESPrino("vecmd ledmode_2_3");
  }
  ip = WiFi.localIP();
}

void waitForWifi(uint16_t timeoutMs) {
  bool putLF = false;
  int delayFix = 100;
  //const static uint32_t timeoutMs =1000L;
  for (int i=0; i*delayFix < timeoutMs; i++) {
    if (WiFi.status() == WL_CONNECTED) return;
    delay(delayFix);
    handleWifi();
    menuHandler.processUserInput();
    if ((i%10) == 0) {
      SERIAL << '.';
      putLF = true;
    }
  }
}


//char x[30], y[30];

int wifiConnectToStoredSSID() {
  String ssid, pass;
  ssid = PropertyList.readProperty(EE_WIFI_SSID);
  pass = PropertyList.readProperty(EE_WIFI_P1);
  SERIAL << F("Connecting to: \"") << ssid << F("\", \"") << pass << F("\"") << endl;
  WiFi.disconnect();
  delay(500);
  WiFi.mode(WIFI_STA);
  // strcpy(x, ssid.c_str());
  // strcpy(y, pass.c_str());
  // if (y[0] == 'B') {
  // //  strcpy(x, "vladiHome");
  //   strcpy(y, "0888414447");
  // }
  // SERIAL << F("Connecting to: \"") << x << F("\", \"") << y << F("\"") << endl;
  WiFi.begin(ssid.c_str(), pass.c_str());
  //WiFi.begin("MarinaResidence", "sdsa");
}
void connectToWifi(const char *s1, const char *s2, const char *s3) {
  PropertyList.putProperty(EE_WIFI_SSID, s1);
  PropertyList.putProperty(EE_WIFI_P1, s2);
  PropertyList.putProperty(EE_WIFI_P2, s3);

  wifiConnectToStoredSSID();
  //SERIAL << "Connecting to " << s1 << endl;
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
  SERIAL.println("scan start");
  WiFi.disconnect();
  delay(500);

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  SERIAL.println("scan done");
  if (n == 0)
    SERIAL.println("no networks found");
  else
  {
    SERIAL.print(n) ;
    SERIAL.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      SERIAL.print(i + 1);
      SERIAL.print(": ");
      SERIAL.print(WiFi.SSID(i));
      SERIAL.print(" (");
      SERIAL.print(WiFi.RSSI(i));
      SERIAL.print(")");
      SERIAL.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
    }
  }
  SERIAL.println("");

  WiFi.mode(WIFI_STA);
  wifiConnectToStoredSSID();

  // Wait a bit before scanning again

}

void setWifi(const char* p) {
  SERIAL << "setWifi_sgtart" << endl;
char s1[30], s2[30], s3[30];
  p = extractStringFromQuotes(p, s1, 29);
  p = extractStringFromQuotes(p, s2, 29);
  p = extractStringFromQuotes(p, s3, 29);
  SERIAL << "setWifi" << s1 << s2 << s3 << endl;

  connectToWifi(s1, s2, s3);
}

void sendPingPort(const char *p) {
  char host[30],  port[20];
  p = extractStringFromQuotes(p, host, 30);
  p = extractStringFromQuotes(p, port, 20);
  int iport = atoi(port);
  WiFiClient ccc;
  SERIAL << "Test connection to to:" << host << ":" << port << endl;
  int res = ccc.connect(host, iport);
  SERIAL << "Res: " << res << endl;

}

void sndIOT(const char *line) {
  if (WiFi.status() != WL_CONNECTED) {
    SERIAL << F("Cannot send data. No Wifi connection.") << endl;
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
  wifiMulti->addAP("Ivan", "4506285842");
  String ssid = PropertyList.readProperty(EE_WIFI_SSID);
  String pass = PropertyList.readProperty(EE_WIFI_P1);
  wifiMulti->addAP(ssid.c_str(), pass.c_str());
}

void WIFI_registerCommands(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("scan"), CMD_EXACT, &wifiScanNetworks, F("")));
  handler->registerCommand(new MenuEntry(F("wifi"), CMD_BEGIN, &setWifi, F("")));
  handler->registerCommand(new MenuEntry(F("ping"), CMD_BEGIN, sendPingPort, F("")));
  handler->registerCommand(new MenuEntry(F("sndiot"), CMD_BEGIN, sndIOT, F("")));
  handler->registerCommand(new MenuEntry(F("sleeptype"), CMD_BEGIN, cmdSleeptype, F("")));
  handler->registerCommand(new MenuEntry(F("delay"), CMD_BEGIN, cmdDelay, F("")));
}
