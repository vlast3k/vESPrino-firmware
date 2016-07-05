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
  } 
  ip = WiFi.localIP();
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

void wifiScanNetworks() {
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
    SERIAL.print(n);
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

