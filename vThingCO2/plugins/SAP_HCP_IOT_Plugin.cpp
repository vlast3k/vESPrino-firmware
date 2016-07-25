#include "common.hpp"
#include "SAP_HCP_IOT_Plugin.hpp"
#include "plugins\AT_FW_Plugin.hpp"
#define COLOR_COUNT 10

void SAP_HCP_IOT_Plugin::registerCommands(MenuHandler *handler) {
   handler->registerCommand(new MenuEntry(F("cfgiot1"), CMD_BEGIN, &SAP_HCP_IOT_Plugin::cfgHCPIOT1, F("HCP Cfg 1")));
   handler->registerCommand(new MenuEntry(F("cfgiot2"), CMD_EXACT, &SAP_HCP_IOT_Plugin::cfgHCPIOT1, F("HCP Cfg 2")));
}

void SAP_HCP_IOT_Plugin::doSend() {
  if (shouldSend == false) return;
  shouldSend = false;
  char tmp[200];
  SERIAL << F("Button Clicked!") << endl;
  if(WiFi.status() != WL_CONNECTED) {
    SERIAL << F("Will not send: No WiFi") << endl;
    return;
  }
  if (!getJSONConfig(SAP_IOT_HOST, tmp)[0]) {
    SERIAL << F("Will not send: No configuration") << endl;
    return;
  }
  HTTPClient http;
  //https://iotmmsi024148trial.hanatrial.ondemand.com/com.sap.iotservices.mms/v1/api/http/push/e46304a8-a410-4979-82f6-ca3da7e43df9
  //{"method":"http", "sender":"My IoT application", "messageType":"42c3546a088b3ef8b8d3", "messages":[{"command":"yellow"}]}
  String rq = String("https://") + getJSONConfig(SAP_IOT_HOST, tmp) + F("/com.sap.iotservices.mms/v1/api/http/data/") + getJSONConfig(SAP_IOT_DEVID, tmp) + "/" + getJSONConfig(SAP_IOT_BTN_MSGID, tmp) + "/sync?button=IA==";
  SERIAL << "Sending: " << rq << endl;
  http.begin(rq);
  http.addHeader("Content-Type",  "application/json;charset=UTF-8");
  //http.setAuthorization("P1940433103", "Abcd1234");
  http.addHeader("Authorization", String("Bearer ") + getJSONConfig(SAP_IOT_TOKEN, tmp));
//  String post = "";
//  post += "{\"method\":\"http\", \"sender\":\"My IoT application\", \"messageType\":\"" + getJSONConfig(SAP_IOT_BTN_MSGID) + "\", \"messages\":[{\"button\":\"" + colors[(clicks++) % COLOR_COUNT] + "\"}]}";
  int httpCode = http.POST("");
  // httpCode will be negative on error
  if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      SERIAL.printf(String(F("[HTTP] GET... code: %d\n")).c_str(), httpCode);
      String payload = http.getString();
      SERIAL.println(payload);
  } else {
      SERIAL.printf(String(F("[HTTP] GET... failed, error: %s\n")).c_str(), http.errorToString(httpCode).c_str());
  }
}



//String colors[] = {"red","pink","lila","violet","blue","mblue","cyan","green","yellow","orange"};


//h iotmmsi024148trial.hanatrial.ondemand.com
//d c5c73d69-6a19-4c7d-9da3-b32198ba71f9
//m 2023a0e66f76d20f47d7
//v co2
//t 46de4fc404221b32054a8405f602fd


//uint32_t lastSAPCheck = -1000000L;
void SAP_HCP_IOT_Plugin::handleSAP_IOT_PushService() {
    char tmp[200];
    if(WiFi.status() != WL_CONNECTED) return;
    heap("");
    //Serial << " before get json config" << endl;
    if (!getJSONConfig(SAP_IOT_HOST, tmp)[0]) return;
    //Serial << " after " << endl;
    String url = String("https://") + getJSONConfig(SAP_IOT_HOST, tmp) + F("/com.sap.iotservices.mms/v1/api/http/data/") + getJSONConfig(SAP_IOT_DEVID, tmp) ;
//        Serial << " before get json config" << end;

    //SERIAL << url << endl;
//    SERIAL << getJSONConfig(SAP_IOT_HOST) << endl;
//    if (getJSONConfig(SAP_IOT_HOST)) SERIAL << "ok" << endl;
//    else SERIAL << "false" << endl;
    //SERIAL << (getJSONConfig(SAP_IOT_HOST) == true) << endl;
    HTTPClient http;
    http.begin(url);
    //Serial << " after begin " << endl;
    http.addHeader("Content-Type",  "application/json;charset=UTF-8");
//    SERIAL <<  getJSONConfig(SAP_IOT_TOKEN) << endl;
    http.addHeader("Authorization", String("Bearer ") + getJSONConfig(SAP_IOT_TOKEN, tmp));
    //SERIAL << "make req" << endl;
 //   Serial << " after begin " << endl;
    int httpCode = http.GET();
    //SERIAL << "make req " << httpCode << endl;
    // httpCode will be negative on error
    if(httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        //SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
        String payload = http.getString();
        //SERIAL.println(payload);
        processMessage(payload);
    } else {
      SERIAL << F("Failed to push message to: ") << url << ", due to: " <<http.errorToString(httpCode) << endl;
        //SERIAL.printf("[HTTP] GET... failed, error: %s, url\n", http.errorToString(httpCode).c_str());
    }

    //lastSAPCheck = millis();
}



void SAP_HCP_IOT_Plugin::cfgHCPIOT1(const char *p) {
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
  printJSONConfig("");

  //heap("");
}

void SAP_HCP_IOT_Plugin::cfgHCPIOT2(const char *p) {
  char buf[140];
    p = extractStringFromQuotes(p, buf, sizeof(buf)); // token
  storeToEE(EE_IOT_TOKN_40B, buf, 40);     // token
  //SERIAL << "IOT OAuth Token: " << buf << endl;
  putJSONConfig(SAP_IOT_TOKEN, buf);

  p = extractStringFromQuotes(p, buf, sizeof(buf)); // button messageid
  //SERIAL << "-" << buf << "-" << endl;
  putJSONConfig(SAP_IOT_BTN_MSGID, buf);
  printJSONConfig("");


  //heap("");
}

void SAP_HCP_IOT_Plugin::sndHCPIOT(const char *line) {
  char host[60], path[140], token[40];
  EEPROM.get(EE_IOT_HOST_60B, host);
  EEPROM.get(EE_IOT_PATH_140B, path);
  EEPROM.get(EE_IOT_TOKN_40B, token);

  sprintf(path, "%s%s", path, &line[7]);
  if (DEBUG) SERIAL << F("Sending to HCP: ") << path << endl;
//  SERIAL << "hcpiot, token: " << token << endl;

  HTTPClient http;
  http.begin(String(HTTPS_STR) + host + path);
  addHCPIOTHeaders(&http, token);
  int rc = AT_FW_Plugin::processResponseCodeATFW(&http, http.POST(""));
  //SERIAL << "IOT rc: " << http.errorToString(rc).c_str();
  //heap("");
}

void SAP_HCP_IOT_Plugin::addHCPIOTHeaders(HTTPClient *http, const char *token) {
  http->addHeader("Content-Type",  "application/json;charset=UTF-8");
  http->addHeader("Authorization", String("Bearer ") + token);
}
