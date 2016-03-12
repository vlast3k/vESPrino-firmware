#ifdef SAP_AUTH

void on302() {
  char str[30];
  EEPROM.get(EE_WIFI_SSID_30B, str);
  if (strstr(str, "SAP-Guest") == 0) {
    checkSAPAuth();
    sendPing();
  }
}

int checkSAPAuth() {
  if (strstr(WiFi.SSID().c_str(), "SAP-Guest") == 0) {
    //SERIAL << "Network is not SAP-Guest : "<< WiFi.SSID() << endl;
    return -1;
  }
  SERIAL << F("Checking SAP Authentication") << endl;
  if (WiFi.status() != WL_CONNECTED) {
    SERIAL << F("Cannot set SAP Guest credentials. No WiFi!") << endl;
    return -1;
  }
  httpAuthSAP();
  if (sendPing() == 302) {
    if (httpAuthSAP() == 302) {
      SERIAL << F("SAP-Guest User/Pass - incorrect") << endl;
      return -1;
    }
    if (sendPing() != 200) {
      SERIAL << F("Could not authenticate for SAP") << endl;
      return -1;
    }
  }
  SERIAL << F("SAP Auth - OK!") << endl;
  return 1;
}

char *sapAuthPreparePostData(char *postData) {
  char userName[30];// = "36121513";
  char pass[30];// = "57cKi5wE";
  EEPROM.get(EE_WIFI_P1_30B, userName);
  EEPROM.get(EE_WIFI_P2_30B, pass);
  if (userName[0] < 2) {
    SERIAL << F("Missing SAP-Guest user/pass") << endl;
    return NULL;
  }
  sprintf(postData, String(F("user=%s&password=%s&cmd=authenticate&url=http%3A%2F%2Fgoogle.bg%2F&Login=Log+In")).c_str(), userName, pass);
  return postData;
}



int httpAuthSAP() {
  char postData[100];
  if ( ! sapAuthPreparePostData(postData) ) return -1;
//  char sapAuthHeaders* = "\"Content-Type\", \"application/x-www-form-urlencoded\",\"Referer\" \"https://emea-guest.wlan.sap.com/guest/sap_guest_register_login.php?_browser=1\"";    
  HTTPClient http;
  http.begin(F("https://securelogin.wlan.sap.com/cgi-bin/login"));
  http.addHeader("Content-Type", String(F("application/x-www-form-urlencoded")).c_str());
  http.addHeader("Referer", String(F("https://emea-guest.wlan.sap.com/guest/sap_guest_register_login.php?_browser=1")).c_str());
  int rc = http.POST((uint8_t*)postData, strlen(postData));
  if (rc < 0 || rc == 200) return 1;
  else return -1; 
//  int rc = sendHTTP("securelogin.wlan.sap.com", "POST", "/cgi-bin/login", sapAuthHeaders, postData, true, true);
//  SERIAL << "SAP Auth Response is: " << rc << endl;
//  if (rc == -1 || rc == 200) return 1;
//  else return -1;
}
  // rc == -1 - timeout connect = connection exists
  // rc == 302 - bad user pass
  // rc == 200 - authenticated
  

int sendPing() {
  HTTPClient http;
  http.begin("http://ping.eu/");
  return processResponseCodeATFW(&http, http.sendRequest("HEAD", (uint8_t*)"", 0));

  //302 - no connection
  //200 - connection ok
}

#endif
