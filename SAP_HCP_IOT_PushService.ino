

//uint32_t lastSAPCheck = -1000000L;
void handleSAP_IOT_PushService() {
    if(WiFi.status() != WL_CONNECTED) return;
    if (!getJSONConfig(SAP_IOT_HOST)) return;
    String url = String("https://") + getJSONConfig(SAP_IOT_HOST) + "/com.sap.iotservices.mms/v1/api/http/data/" + getJSONConfig(SAP_IOT_DEVID) ; 
//    SERIAL << url << endl;
//    SERIAL << getJSONConfig(SAP_IOT_HOST) << endl;
//    if (getJSONConfig(SAP_IOT_HOST)) SERIAL << "ok" << endl;
//    else SERIAL << "false" << endl;
    //SERIAL << (getJSONConfig(SAP_IOT_HOST) == true) << endl;
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type",  "application/json;charset=UTF-8");
//    SERIAL <<  getJSONConfig(SAP_IOT_TOKEN) << endl;
    http.addHeader("Authorization", String("Bearer ") + getJSONConfig(SAP_IOT_TOKEN));  
  //  SERIAL << "make req" << endl;
    int httpCode = http.GET();
    // httpCode will be negative on error
    if(httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        //SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
        String payload = http.getString();
        //SERIAL.println(payload);          
        processMessage(payload);
    } else {
      SERIAL << "Failed to push message to: " << url << ", due to: " <<http.errorToString(httpCode) << endl;
        //SERIAL.printf("[HTTP] GET... failed, error: %s, url\n", http.errorToString(httpCode).c_str());
    }

    //lastSAPCheck = millis();
}

//void httpGET(String url) {
//    if(WiFi.status() == WL_CONNECTED) {
//        HTTPClient http;
//
//        SERIAL.print("[HTTP] begin...\n");
//        http.begin(url); //HTTP
//
//        SERIAL.print("[HTTP] GET...\n");
//        // start connection and send HTTP header
//        int httpCode = http.GET();
//
//        // httpCode will be negative on error
//        if(httpCode > 0) {
//            // HTTP header has been send and Server response header has been handled
//            SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);
//            String payload = http.getString();
//            SERIAL.println(payload);          
//        } else {
//            SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
//        }
//
//        http.end();
//    }  
//}

void processMessage(String msgIn) {
  StaticJsonBuffer<200> jsonBuffer;
  char msg[301];
  strncpy(msg, msgIn.c_str(), 300);
  JsonArray& root = jsonBuffer.parseArray(msg);
  //[{"messageType":"42c3546a088b3ef8b8d3","sender":"IoT App","messages":[{"command":"switch on"}]}]

  if (!root.success()) {
    //if server has returned empyy response
    SERIAL << "parseObject() failed: " << msgIn << endl;
    return;
  }
  //SERIAL.print(root[0].is<JsonObject&>());
  //SERIAL  << "type:" << root.get(0) << endl;
  if (root[0].is<JsonObject&>()) {
    //SERIAL << root[0].asObject().containsKey("messages") << endl; 
    //SERIAL << root[0]["messages"].is<JsonArray&>() << endl;
    //SERIAL << root[0]["messages"][0].is<JsonObject&>() << endl;
    //SERIAL << root[0]["messages"][0].asObject().containsKey("color") << endl;
    //SERIAL << " Received cmd: " << root[0]["messages"][0]["color"].asString() << endl;
    if (root[0]["messages"][0]["color"].asString()) {
      //SERIAL << "to process command" << endl;
      processCommand(root[0]["messages"][0]["color"].asString());
    } else {
      SERIAL << "command not recognized" << endl;
     }
  } 
//  else {
//    SERIAL << "no cmd" << endl;
//  }

}

