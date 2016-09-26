#include "common.hpp"
#include "plugins\SAP_HCP_IOT_Plugin.hpp"
#include "plugins\CustomURL_Plugin.hpp"
#include "plugins\PropertyList.hpp"
#include <ESP8266WiFiMulti.h>
#include <WiFiManager.h>

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
  if (WiFi.status() == WL_CONNECTED)  return WL_CONNECTED;
  Serial << F("Waiting for WiFi ");
  //bool putLF = false;
  int delayFix = 100;
  //const static uint32_t timeoutMs =1000L;
  for (int i=0; i*delayFix < timeoutMs; i++) {
    if (WiFi.status() == WL_CONNECTED) break;
    delay(delayFix);
    handleWifi();
    menuHandler.loop();
    if ((i%10) == 0) SERIAL_PORT << '.';
  }
  Serial << endl;
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
  char s1[50], s2[130], s3[30];
  p = extractStringFromQuotes(p, s1, sizeof(s1));
  p = extractStringFromQuotes(p, s2, sizeof(s2));
  p = extractStringFromQuotes(p, s3, sizeof(s3));
  connectToWifi(s1, s2, s3);
}

void sendPingPort(const char *p) {
  char host[30],  port[20];
  p = extractStringFromQuotes(p, host, 30);
  p = extractStringFromQuotes(p, port, 20);
  int iport = atoi(port);
  WiFiClient ccc;
  SERIAL_PORT << F("Test connection to to:") << host << F(":") << port << endl;
  int res = ccc.connect(host, iport);
  SERIAL_PORT << F("Res: ") << res << endl;

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
  Serial << F("set sleep type to:") << type<< endl;
  switch (type) {
    case 0: WiFi.setSleepMode(WIFI_NONE_SLEEP);
    case 1: WiFi.setSleepMode(WIFI_MODEM_SLEEP);
    case 2: WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
  }
}

void cmdDelay(const char *line) {
  int d = atoi(strchr(line, ' ') + 1);
  Serial << F("delay for:") << d<< endl;
  delay(d);
}

void wifiConnectMulti() {
  if (wifiMulti) delete wifiMulti;
  WiFi.mode(WIFI_STA);
  wifiMulti = new ESP8266WiFiMulti();
  wifiMulti->addAP("vladiHome", "0888414447");
  wifiMulti->addAP("Andreev", "4506285842");
  String ssid = PropertyList.readProperty(EE_WIFI_SSID);
  String pass = PropertyList.readProperty(EE_WIFI_P1);
  WiFi.persistent(false);
  if (ssid.length() && ssid.length() < 40 && pass.length() < 100) {
    wifiMulti->addAP(ssid.c_str(), pass.c_str());
    char x[120], y[120];
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

extern NeopixelVE neopixel;
void startAutoWifiConfig() {
  char custom_http1[140] = "Custom HTTP URL 1";
  char custom_http2[140] = "Custom HTTP URL 2";
  char custom_http3[140] = "Custom HTTP URL 3";
  char mqtt_server[40] = "mqtt_server";
  char mqtt_port[6] = "1883";
  char mqtt_msg1[140] = "<mqtt topic>:<msg1>";
  char mqtt_msg2[140] = "<mqtt topic>:<msg2>";
  char mqtt_msg3[140] = "<mqtt topic>:<msg3>";
  shouldSend = false;
  WiFiManager wifiManager;
  WiFiManagerParameter par_custom_http1("http1", "http1", custom_http1, 140);
  WiFiManagerParameter par_custom_http2("http2", "http2", custom_http2, 140);
  WiFiManagerParameter par_custom_http3("http3", "http3", custom_http3, 140);
  WiFiManagerParameter par_custom_mqtt_server("mqttserver", "mqtt server", mqtt_server, 40);
  WiFiManagerParameter par_custom_mqtt_port("port", "mqtt port", mqtt_port, 6);
  WiFiManagerParameter par_custom_mqtt_msg1("mqmsg1", "mqmsg1", mqtt_msg1, 140);
  WiFiManagerParameter par_custom_mqtt_msg2("mqmsg2", "mqmsg2", mqtt_msg2, 140);
  WiFiManagerParameter par_custom_mqtt_msg3("mqmsg3", "mqmsg3", mqtt_msg3, 140);
  wifiManager.addParameter(&par_custom_http1);
  wifiManager.addParameter(&par_custom_http2);
  wifiManager.addParameter(&par_custom_http3);
  wifiManager.addParameter(&par_custom_mqtt_server);
  wifiManager.addParameter(&par_custom_mqtt_port);
  wifiManager.addParameter(&par_custom_mqtt_msg1);
  wifiManager.addParameter(&par_custom_mqtt_msg2);
  wifiManager.addParameter(&par_custom_mqtt_msg3);
  neopixel.cmdLedSetBrgInst(F("ledbrg 90"));
  neopixel.cmdLedHandleColorInst(F("ledcolor blue"));

  WiFi.persistent(false);
  wifiManager.setConnectTimeout(10);
  wifiManager.autoConnect("vAirMonitor");

  menuHandler.handleCommand(F("custom_url_clean"));
  if (par_custom_http1.getValue()[0]) menuHandler.handleCommand((String(F("custom_url_add \"0\",\"")) + par_custom_http1.getValue() + "\"").c_str());
  if (par_custom_http2.getValue()[0]) menuHandler.handleCommand((String(F("custom_url_add \"1\",\"")) + par_custom_http2.getValue() + "\"").c_str());
  if (par_custom_http3.getValue()[0]) menuHandler.handleCommand((String(F("custom_url_add \"2\",\"")) + par_custom_http3.getValue() + "\"").c_str());
  if (par_custom_mqtt_server.getValue()[0]) {
    char x[200];
    sprintf(x, String(F("mqtt_setup \"%s\",\"%s\",\"\",\"\",\"\"")).c_str(), par_custom_mqtt_server.getValue(), par_custom_mqtt_port.getValue());
    menuHandler.handleCommand(x);
  }
  menuHandler.handleCommand(F("mqtt_msg_clean"));
  if (par_custom_mqtt_msg1.getValue()[0]) menuHandler.handleCommand((String(F("mqtt_msg_add \"0\"")) + par_custom_mqtt_msg1.getValue()).c_str());
  if (par_custom_mqtt_msg2.getValue()[0]) menuHandler.handleCommand((String(F("mqtt_msg_add \"1\"")) + par_custom_mqtt_msg2.getValue()).c_str());
  if (par_custom_mqtt_msg3.getValue()[0]) menuHandler.handleCommand((String(F("mqtt_msg_add \"2\"")) + par_custom_mqtt_msg3.getValue()).c_str());

  PropertyList.putProperty(EE_WIFI_SSID, WiFi.SSID().c_str());
  PropertyList.putProperty(EE_WIFI_P1, WiFi.psk().c_str());
  menuHandler.handleCommand(F("prop_list"));
  neopixel.cmdLedSetBrgInst(F("ledbrg 99"));
  neopixel.cmdLedHandleColorInst(F("ledcolor lila"));
  //Serial << "wifiaaaaaaaaa: " << WiFi.SSID() << " : " << WiFi.psk() << endl;
}
