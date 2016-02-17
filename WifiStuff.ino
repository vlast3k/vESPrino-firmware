 IPAddress ip = WiFi.localIP();

void handleWifi() {
  //Serial << "ip = " << ip  << ", localip:" << WiFi.localIP() << endl;
  if (ip == WiFi.localIP()) return;
  else if (WiFi.status() == WL_CONNECTED) {
    ip = WiFi.localIP();
    checkSAPAuth();
    Serial << "IP address: " << WiFi.localIP() << " in " << millis() << " ms" << endl << "GOT IP" << endl; 
  } 
  ip = WiFi.localIP();
}

void connectToWifi(const char *s1, const char *s2, const char *s3) {
  storeToEE(EE_WIFI_SSID_30B, s1);
  storeToEE(EE_WIFI_P1_30B, s2);
  storeToEE(EE_WIFI_P2_30B, s3);

  wifiConnectToStoredSSID();
  //Serial << "Connecting to " << s1 << endl;
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
  Serial << "Connecting to: \"" << ssid << "\", \"" << pass << "\"" << endl;
  WiFi.disconnect();
  delay(500);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
}

void wifiScanNetworks() {
  Serial.println("scan start");
  WiFi.disconnect();
  delay(500);

  // WiFi.scanNetworks will return the number of networks found
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  else
  {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i)
    {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (");
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE)?" ":"*");
      delay(10);
    }
  }
  Serial.println("");

  // Wait a bit before scanning again

}

