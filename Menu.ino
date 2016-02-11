#define LINE_LEN 200
char line[LINE_LEN];

void processUserInput() {
  Serial.setTimeout(500);
  if (!Serial.available()) return;
  
  Serial.setTimeout(30000);
  if (readLine(30000) >= 0) {
    Serial.flush();
    handleCommand();
   // Serial << endl << F("OK") << endl;
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

int handleCommand() {
  Serial << "Received command: " << line << endl;
  if (line[0] == 't') sendTS();
  else if (line[0] == 'p') sendPing();
  else if (line[0] == 'A') mockATCommand(line);
  else if (line[0] == 'S') httpAuthSAP();
  else if (line[0] == 'C') checkSAPAuth();
  else if (line[0] == 'G') getTS(line);
  else if (strstr(line, "cfggen")) cfgGENIOT(line);
  else if (strstr(line, "cfgiot")) cfgHCPIOT(line);
  else if (strstr(line, "sndiot")) sndIOT(line);
  //else if (strstr(line, "smp2")) sndSimple2();
  else if (strstr(line, "smp")) sndSimple();
  else if (strstr(line, "wifi")) setWifi(line);
  else if (strstr(line, "scan")) wifiScanNetworks();
  else if (strstr(line, "otahtest")) doHttpUpdate(2);
  else if (strstr(line, "otah")) doHttpUpdate(1);
  else if (line[0] == 'o') startOTA();
  else if (strstr(line, "ubi")) testUBI();
  return 0;
}

char atCIPSTART_IP[20];
void getTS(const char* line) {
  HTTPClient http;
  http.begin(HTTP_STR  + atCIPSTART_IP + (line + 4));
  processResponseCodeATFW(&http, http.GET());
}

void testUBI() {
  HTTPClient http;
  http.begin("http://50.23.124.66/api/postvalue/?token=Cg5W22qmWFcsMqsALMik04VtEF7PYA&variable=565965867625420c74ec604b&value=456");
  processResponseCodeATFW(&http, http.GET());  
}

void sendTS() {
  HTTPClient http;
  http.begin("http://api.thingspeak.com/update?key=2DB818ODLIFO8TLF&field1=456");
  processResponseCodeATFW(&http, http.GET());
}

int setWifi(const char* p) {
  char s1[20], s2[20], s3[20];
  p = extractStringFromQuotes(p, s1, 20);
  p = extractStringFromQuotes(p, s2, 20);
  p = extractStringFromQuotes(p, s3, 20);
  Serial << "setWifi" << s1 << s2 << s3 << endl;

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
    
    if (strstr(line, "AT+CIPSEND"))  Serial << ">" << endl; 
    else                             Serial << "OK" << endl;
  }
}

void cfgGENIOT(const char *p) {
  char genurl[140] = "";
  if (!p[6]) {
    Serial << "Cleared Generic URL" << endl;    
  } else {
    strncpy(genurl, p+7, sizeof(genurl)-1);
    Serial << "Stored Generic URL: " << genurl << endl;
  }
  storeToEE(EE_GENIOT_PATH_140B, genurl); // path
  Serial << "DONE" << endl;
}

void cfgHCPIOT(const char *p) {
  //POST https://iotmmsi024148trial.hanatrial.ondemand.com/com.sap.iotservices.mms/v1/api/http/data/c5c73d69-6a19-4c7d-9da3-b32198ba71f9/2023a0e66f76d20f47d7/sync?co2=34
  // host: iotmmsi024148trial.hanatrial.ondemand.com
  // deviceId: c5c73d69-6a19-4c7d-9da3-b32198ba71f9
  // messageId: 2023a0e66f76d20f47d7
  // variable name: co2
  
  // Authorization: Bearer 46de4fc404221b32054a8405f602fd

  char buf[140], devId[40], msgId[25], varName[20];
  p = extractStringFromQuotes(p, buf, sizeof(buf)); // host
  storeToEE(EE_IOT_HOST_60B, buf);     //host
  Serial << "IOT Host: " << buf << endl;
  
  p = extractStringFromQuotes(p, devId, sizeof(devId)); 
  p = extractStringFromQuotes(p, msgId, sizeof(msgId)); 
  p = extractStringFromQuotes(p, varName, sizeof(varName)); 
  sprintf(buf, "/com.sap.iotservices.mms/v1/api/http/data/%s/%s/sync?%s=", devId, msgId, varName);
  storeToEE(EE_IOT_PATH_140B, buf); // path
  Serial << "IOT Path: " << buf << endl;
  
  p = extractStringFromQuotes(p, buf, sizeof(buf)); // token
  storeToEE(EE_IOT_TOKN_40B, buf);     // token
  Serial << "IOT OAuth Token: " << buf << endl;
  heap("");
}

void sndIOT(const char *line) {
  Serial << "Send Value" << endl;
  char path[140];
  EEPROM.get(EE_IOT_PATH_140B, path);
  if (path[0] && path[0] != 255) {
    sndHCPIOT(line);    
  } 
  
  EEPROM.get(EE_GENIOT_PATH_140B, path);
  if (path[0] && path[0] != 255) {
    sndGENIOT(line);
  } 
}

void sndGENIOT(const char *line) {
  char str[140], str2[150];
  EEPROM.get(EE_GENIOT_PATH_140B, str);
  Serial << "geniot: " << str << endl;
  sprintf(str2, str, &line[7]);
  Serial << "geniot: " << str2 << endl;
  
  HTTPClient http;
  http.begin(str2);
  //addHCPIOTHeaders(&http, token);
  int rc = processResponseCodeATFW(&http, http.GET());
  Serial << "GEN rc: " << rc;
}

void sndHCPIOT(const char *line) {
  char host[60], path[140], token[40];
  EEPROM.get(EE_IOT_HOST_60B, host);
  EEPROM.get(EE_IOT_PATH_140B, path);
  EEPROM.get(EE_IOT_TOKN_40B, token);

  sprintf(path, "%s%s", path, &line[7]);
  Serial << "hcpiot: " << path << endl;
  
  HTTPClient http;
  http.begin(HTTPS_STR + host + path);
  addHCPIOTHeaders(&http, token);
  int rc = processResponseCodeATFW(&http, http.POST(""));
  Serial << "IOT rc: " << rc;
  heap("");
}

void addHCPIOTHeaders(HTTPClient *http, const char *token) {
  http->addHeader("Content-Type",  "application/json;charset=UTF-8");
  http->addHeader("Authorization", String("Bearer ") + token);  
}

void sndSimple() {
  HTTPClient http;
  http.begin("https://iotmmsi024148trial.hanatrial.ondemand.com/com.sap.iotservices.mms/v1/api/http/data/c5c73d69-6a19-4c7d-9da3-b32198ba71f9/2023a0e66f76d20f47d7/sync?co2=34");
  addHCPIOTHeaders(&http, "be4e6b1381f6989b195a402420399a8");
  processResponseCodeATFW(&http, http.POST(""));
}


