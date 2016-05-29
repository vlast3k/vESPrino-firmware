#define LINE_LEN 250
char line[LINE_LEN];
String HTTP_STR = "http://";
String HTTPS_STR= "https://";


void sendPingPort(const char *p);
boolean processUserInput() {
  Serial.setTimeout(500);
  if (!Serial.available()) {
    return false;
  }
  
  Serial.setTimeout(30000);
  if (readLine(30000) >= 0) {
    Serial.flush();
    handleCommand();
   // SERIAL << endl << F("OK") << endl;
   return true;
  }
}

byte readLine(int timeout) {
  unsigned long deadline = millis() + timeout;
  byte i = 0;
  while (millis() < deadline) {
    if (Serial.available()) {
      line[i++] = (char) Serial.read();
      if      (line[i-1] == '\r')  i--;   
      else if (i == LINE_LEN - 1)  break; 
      else if (line[i-1] == '\n')  {i--; break;}
    }
  }
  line[i] = 0;
  return i;
}

void dumpCfg();
String ubik;
int handleCommand() {
  if (DEBUG) SERIAL << "Received command: " << line << endl;
  if (strstr(line, "tstest")) sendTS();
  else if (line[0] == 'A') mockATCommand(line);
#ifdef SAP_AUTH
  else if (strcmp(line, "ping") == 0) vSAP_Auth(EE_WIFI_P1_30B, EE_WIFI_P1_30B).sendPing();
  else if (line[0] == 'S') vSAP_Auth(EE_WIFI_P1_30B, EE_WIFI_P1_30B);
  else if (line[0] == 'C') vSAP_Auth(EE_WIFI_P1_30B, EE_WIFI_P1_30B);
#endif
#ifndef VTHING_H801_LED
  else if (line[0] == 'G') getTS(line);
  else if (strstr(line, "cfggen")) cfgGENIOT(line);
  else if (strstr(line, "cfgiot1")) cfgHCPIOT1(line);
  else if (strstr(line, "cfgiot2")) cfgHCPIOT2(line);
  else if (strstr(line, "sndiot")) sndIOT(line);
  //else if (strstr(line, "smp2")) sndSimple2();
  else if (strstr(line, "smp")) sndSimple();
  else if (strcmp(line, "test") == 0) sndIOT("sndiot 567");
  else if (strcmp(line, "ubi") == 0) testUBI();
  else if (strstr(line, "tskey ")) cfgGENIOT((String("cfggen http://api.thingspeak.com/update?key=") + &line[6] + "&field1=%s").c_str());
  else if (strstr(line, "ubik "))  ubik = String(&line[5]);  
  else if (strstr(line, "ubiv "))  cfgGENIOT((String("cfggen http://50.23.124.66/api/postvalue/?token=") + ubik + "&variable=" + &line[5] + "&value=%s").c_str());
  else if (strstr(line, "jjj")) testJSON();
  else if (strstr(line, "scan")) wifiScanNetworks();
  else if (strstr(line, "atest_mqtt")) sendMQTT("556");
  else if (strstr(line, "bttn")) shouldSend=true;
#endif
#ifdef VTHING_CO2
  else if (strstr(line, "wsi ")) setSendInterval (line);
  else if (strstr(line, "wst ")) setSendThreshold(line);
  else if (strcmp(line, "rco") == 0) resetCO2();
#endif
  else if (strstr(line, "wifi")) setWifi(line);
  else if (strcmp(line, "otahtest") == 0) doHttpUpdate(2, NULL);
  else if (strcmp(line, "otah") == 0) doHttpUpdate(1, NULL);
  else if (strstr(line, "otau"))      doHttpUpdate(0, &line[5]);
  //else if (strcmp(line, "otaa") == 0)  startOTA();
  else if (strstr(line, "cfg_mqtt"))  configMQTT(line);
  else if (strstr(line, "cfg_mqval"))  { storeToEE(EE_MQTT_VALUE_70B, &line[10], 70); SERIAL << "DONE" << endl; }
  else if (strstr(line, "info")) printVersion();
  else if (strstr(line, "jscfg")) printJSONConfig();
  else if (strcmp(line, "restart") == 0) ESP.restart();
  else if (strcmp(line, "factory") == 0) factoryReset();
#ifdef VTHING_H801_LED
  else if (strcmp(line, "testled") == 0) testH801();
  else if (strstr(line, "h801cfg")) h801_processConfig(line);
#endif
  else if (strcmp(line, "debug") == 0) DEBUG = true;
  else if (strcmp(line, "thu") == 0) testHttpUpdate();
  else if (strcmp(line, "sss") == 0) {SERIAL << "STOP Active execution\n"; SKIP_LOOP = true; }
  else if (strstr(line, "ping")) sendPingPort(line);
  else if (strcmp(line, "dumpCfg") == 0) dumpCfg();
  else if (strcmp(line, "heap") == 0) heap("");
  
  
  SERIAL << "OK >" << endl;
  return 0;
}

void dumpCfg() {
  for (int i=0; i < 30; i++) {
    SERIAL << i << " : " ;
    for (int j=0; j < 100; j++) {
      byte b = EEPROM.read(i*100 + j);
      if (b == 0) b = '?';
      SERIAL << (char)b;
    }
    delay(10);
    SERIAL << endl;
  }
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

void testHttpUpdate() {
  heap("");
  SERIAL << F("Start http update test") << endl;
    HTTPClient http;
    http.begin(F("https://raw.githubusercontent.com/vlast3k/vThingCO2/master/fw/latest_h801.bin"));
    http.useHTTP10(true);
    http.setTimeout(8000);
    http.setUserAgent(String(F("ESP8266-http-Update")).c_str());
    http.addHeader(F("x-ESP8266-STA-MAC"), WiFi.macAddress());
    http.addHeader(F("x-ESP8266-AP-MAC"), WiFi.softAPmacAddress());
    http.addHeader(F("x-ESP8266-free-space"), String(ESP.getFreeSketchSpace()));
    http.addHeader(F("x-ESP8266-sketch-size"), String(ESP.getSketchSize()));
    http.addHeader(F("x-ESP8266-chip-size"), String(ESP.getFlashChipRealSize()));
    http.addHeader(F("x-ESP8266-sdk-version"), ESP.getSdkVersion());
    int code = http.GET();

    if(code <= 0) {
        SERIAL << F("[httpUpdate] HTTP error: ") <<  http.errorToString(code) << endl;
    } else {
      SERIAL << "OK!" << endl;
    }
        http.end();
}

#ifdef VTHING_CO2
void setSendInterval (const char *line) {
  int interval = 120;
  if (strchr(line, ' ')) {
    interval = atoi(strchr(line, ' ') + 1);
  }
  putJSONConfig(XX_SND_INT, String(interval).c_str());
  intCO2SendValue = (uint32_t)interval * 1000;
  tmrCO2SendValueTimer->setInterval(intCO2SendValue); 
  Serial << "Send Interval (ms): " << intCO2SendValue << endl;
}

void setSendThreshold(const char *line) {
  int thr = 0;
  if (strchr(line, ' ')) {
    thr = atoi(strchr(line, ' ') + 1);
  }
  putJSONConfig(XX_SND_THR, String(thr).c_str());
  co2Threshold = thr;
  Serial << F("CO2 Threshold (ppm): ") << co2Threshold << endl;
}

#endif

char atCIPSTART_IP[20];
void getTS(const char* line) {
  HTTPClient http;
  http.begin(HTTP_STR  + atCIPSTART_IP + (line + 4));
  processResponseCodeATFW(&http, http.GET());
}

void testUBI() {
  HTTPClient http;
  http.begin(F("http://50.23.124.66/api/postvalue/?token=Cg5W22qmWFcsMqsALMik04VtEF7PYA&variable=565965867625420c74ec604b&value=456"));
  processResponseCodeATFW(&http, http.GET());  
}

void sendTS() {
  HTTPClient http;
  http.begin(F("http://api.thingspeak.com/update?key=2DB818ODLIFO8TLF&field1=456"));
  processResponseCodeATFW(&http, http.GET());
}

int setWifi(const char* p) {
  char s1[30], s2[30], s3[30];
  p = extractStringFromQuotes(p, s1, 29);
  p = extractStringFromQuotes(p, s2, 29);
  p = extractStringFromQuotes(p, s3, 29);
  //SERIAL << "setWifi" << s1 << s2 << s3 << endl;

  connectToWifi(s1, s2, s3);
  return 0;
}


void atCIPSTART(const char *p) {
  p = extractStringFromQuotes(p, atCIPSTART_IP, 20);
  p = extractStringFromQuotes(p, atCIPSTART_IP, 20);  
}

void mockATCommand(const char *line) {
  if (line[0] == 'A') {
    if (strstr(line, "AT+CWJAP_DEF")) setWifi(line);
    if (strstr(line, "AT+CIPSTART")) atCIPSTART(line);
    
    if (strstr(line, "AT+CIPSEND"))  SERIAL << ">" << endl; 
    else                             SERIAL << "OK" << endl;
  }
}

void cfgGENIOT(const char *p) {
  char genurl[140] = "";
  if (!p[6]) {
    SERIAL << F("Cleared Generic URL") << endl;    
  } else {
    strncpy(genurl, p+7, sizeof(genurl)-1);
    SERIAL << F("Stored Generic URL: ") << genurl << endl;
  }
  storeToEE(EE_GENIOT_PATH_140B, genurl, 140); // path
  SERIAL << F("DONE") << endl;
}

void cfgHCPIOT1(const char *p) {
  //POST https://iotmmsi024148trial.hanatrial.ondemand.com/com.sap.iotservices.mms/v1/api/http/data/c5c73d69-6a19-4c7d-9da3-b32198ba71f9/2023a0e66f76d20f47d7/sync?co2=34
  // host: iotmmsi024148trial.hanatrial.ondemand.com
  // deviceId: c5c73d69-6a19-4c7d-9da3-b32198ba71f9
  // messageId: 2023a0e66f76d20f47d7
  // variable name: co2
  
  // Authorization: Bearer 46de4fc404221b32054a8405f602fd

  char buf[140], devId[40], msgId[25], varName[20];
  p = extractStringFromQuotes(p, buf, sizeof(buf)); // host
  storeToEE(EE_IOT_HOST_60B, buf, 60);     //host
  putJSONConfig(SAP_IOT_HOST, buf);
  //SERIAL << "IOT Host: " << buf << endl;
  
  p = extractStringFromQuotes(p, devId, sizeof(devId)); 
  putJSONConfig(SAP_IOT_DEVID, devId);
  p = extractStringFromQuotes(p, msgId, sizeof(msgId)); 
  p = extractStringFromQuotes(p, varName, sizeof(varName)); 
  sprintf(buf, String(F("/com.sap.iotservices.mms/v1/api/http/data/%s/%s/sync?%s=")).c_str(), devId, msgId, varName);
  storeToEE(EE_IOT_PATH_140B, buf, 140); // path
  //SERIAL << "IOT Path: " << buf << endl;  
  printJSONConfig();

  //heap("");
}

void cfgHCPIOT2(const char *p) {  
  char buf[140];
    p = extractStringFromQuotes(p, buf, sizeof(buf)); // token
  storeToEE(EE_IOT_TOKN_40B, buf, 40);     // token
  //SERIAL << "IOT OAuth Token: " << buf << endl;
  putJSONConfig(SAP_IOT_TOKEN, buf);

  p = extractStringFromQuotes(p, buf, sizeof(buf)); // button messageid
  //SERIAL << "-" << buf << "-" << endl;
  putJSONConfig(SAP_IOT_BTN_MSGID, buf);
  printJSONConfig();
  

  //heap("");
}


void sndIOT(const char *line) {
  if (WiFi.status() != WL_CONNECTED) {
    SERIAL << F("Cannot send data. No Wifi connection.") << endl;
    return;
  }
  char path[140];
  EEPROM.get(EE_IOT_PATH_140B, path);
  if (path[0] && path[0] != 255) {
    sndHCPIOT(line);    
  } 
  
  EEPROM.get(EE_GENIOT_PATH_140B, path);
  if (path[0] && path[0] != 255) {
    sndGENIOT(line);
  } 

  EEPROM.get(EE_MQTT_SERVER_30B, path);
  if (path[0] && path[0] != 255) {
    sendMQTT(&line[7]);
  } 
}

void sndGENIOT(const char *line) {
  char str[140], str2[150];
  EEPROM.get(EE_GENIOT_PATH_140B, str);
  sprintf(str2, str, &line[7]);
  SERIAL << F("Sending to URL: \"") << str2 << "\"" << endl;
  
  HTTPClient http;
  http.begin(str2);
  //addHCPIOTHeaders(&http, token);
  int rc = processResponseCodeATFW(&http, http.GET());
  //SERIAL << "Result: " << http.errorToString(rc).c_str();
}

void sndHCPIOT(const char *line) {
  char host[60], path[140], token[40];
  EEPROM.get(EE_IOT_HOST_60B, host);
  EEPROM.get(EE_IOT_PATH_140B, path);
  EEPROM.get(EE_IOT_TOKN_40B, token);

  sprintf(path, "%s%s", path, &line[7]);
  if (DEBUG) SERIAL << F("Sending to HCP: ") << path << endl;
//  SERIAL << "hcpiot, token: " << token << endl;
  
  HTTPClient http;
  http.begin(HTTPS_STR + host + path);
  addHCPIOTHeaders(&http, token);
  int rc = processResponseCodeATFW(&http, http.POST(""));
  //SERIAL << "IOT rc: " << http.errorToString(rc).c_str();
  //heap("");
}

void addHCPIOTHeaders(HTTPClient *http, const char *token) {
  http->addHeader("Content-Type",  "application/json;charset=UTF-8");
  http->addHeader("Authorization", String("Bearer ") + token);  
}

void sndSimple() {
  HTTPClient http;
  http.begin(F("https://iotmmsi024148trial.hanatrial.ondemand.com/com.sap.iotservices.mms/v1/api/http/data/c5c73d69-6a19-4c7d-9da3-b32198ba71f9/2023a0e66f76d20f47d7/sync?co2=34"));
  addHCPIOTHeaders(&http, String(F("be4e6b1381f6989b195a402420399a8")).c_str());
  processResponseCodeATFW(&http, http.POST(""));
}

void factoryReset() {
  SERIAL << F("Doing Factory Reset, and restarting...") << endl;
  for (int i=0; i < 3000; i++) EEPROM.write(i, 0xFF);
  EEPROM.commit();  
  ESP.restart();
}


int processResponseCodeATFW(HTTPClient *http, int rc) {
  if (rc > 0) SERIAL << F("Response Code: ") << rc << endl;
  else SERIAL << F("Error Code: ") << rc << " = " << http->errorToString(rc).c_str() << endl;
  if (rc > 0) {
    if (DEBUG) SERIAL << F("Payload: [") << http->getString() << "]" << endl;
    SERIAL << F("CLOSED") << endl; // for compatibility with AT FW
  } else {
    SERIAL << F("Failed") << endl;
  }
  return rc;
}



