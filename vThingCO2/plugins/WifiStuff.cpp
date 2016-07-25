#include "common.hpp"
#include "plugins\SAP_HCP_IOT_Plugin.hpp"
#include "plugins\CustomURL_Plugin.hpp"

 IPAddress ip = WiFi.localIP();

void handleWifi() {
  //SERIAL << "ip = " << ip  << ", localip:" << WiFi.localIP() << endl;
  if (ip == WiFi.localIP()) return;
  else if (WiFi.status() == WL_CONNECTED) {
    ip = WiFi.localIP();
#ifdef SAP_AUTH
    vSAP_Auth(EE_WIFI_P1_30B, EE_WIFI_P1_30B);
#endif
    SERIAL << "IP address: " << WiFi.localIP() << " in " << millis() << " ms" << endl << "GOT IP" << endl;
    // handleCommandVESPrino("vecmd led_mblue");
    // handleCommandVESPrino("vecmd ledmode_2_3");
  }
  ip = WiFi.localIP();
}

int wifiConnectToStoredSSID() {
  char ssid[30], pass[30];
  EEPROM.get(EE_WIFI_SSID_30B, ssid);
  EEPROM.get(EE_WIFI_P1_30B, pass);
  SERIAL << "Connecting to: \"" << ssid << "\", \"" << pass << "\"" << endl;
  WiFi.disconnect();
  delay(500);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
}
void connectToWifi(const char *s1, const char *s2, const char *s3) {
  storeToEE(EE_WIFI_SSID_30B, s1, 30);
  storeToEE(EE_WIFI_P1_30B, s2, 30);
  storeToEE(EE_WIFI_P2_30B, s3, 30);

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
  char s1[30], s2[30], s3[30];
  p = extractStringFromQuotes(p, s1, 29);
  p = extractStringFromQuotes(p, s2, 29);
  p = extractStringFromQuotes(p, s3, 29);
  //SERIAL << "setWifi" << s1 << s2 << s3 << endl;

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
  char path[140];
  EEPROM.get(EE_IOT_PATH_140B, path);
  if (path[0] && path[0] != 255) {
    SAP_HCP_IOT_Plugin::sndHCPIOT(line);
  }

  EEPROM.get(EE_GENIOT_PATH_140B, path);
  if (path[0] && path[0] != 255) {
    CustomURL_Plugin::sndGENIOT(line);
  }

  EEPROM.get(EE_MQTT_SERVER_30B, path);
  if (path[0] && path[0] != 255) {
    sendMQTT(&line[7]);
  }
}


void WIFI_registerCommands(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("scan"), CMD_EXACT, &wifiScanNetworks, F("")));
  handler->registerCommand(new MenuEntry(F("wifi"), CMD_BEGIN, &setWifi, F("")));
  handler->registerCommand(new MenuEntry(F("ping"), CMD_BEGIN, sendPingPort, F("")));
  handler->registerCommand(new MenuEntry(F("sndiot"), CMD_BEGIN, sndIOT, F("")));
}
