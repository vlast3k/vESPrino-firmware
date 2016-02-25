#include <ArduinoJson.h>


uint32_t lastSAPCheck = -1000000L;
void handleSAP_IOT_PushService() {
    if  (millis() - lastSAPCheck < 2000) return;
    String url = String("https://" + mmsHost + "/com.sap.iotservices.mms/v1/api/http/data/" + deviceId); 
    HTTPClient http;
    http.begin(url);
    http.addHeader("Content-Type",  "application/json;charset=UTF-8");
    http.addHeader("Authorization", String("Bearer ") + authToken);  
    
    int httpCode = http.GET();
    // httpCode will be negative on error
    if(httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        String payload = http.getString();
        Serial.println(payload);          
        processMessage(payload);
    } else {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    }

    lastSAPCheck = millis();
}

void httpGET(String url) {
    if(WiFi.status() == WL_CONNECTED) {
        HTTPClient http;

        Serial.print("[HTTP] begin...\n");
        http.begin(url); //HTTP

        Serial.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();

        // httpCode will be negative on error
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
            String payload = http.getString();
            Serial.println(payload);          
        } else {
            Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
    }  
}

void processMessage(String msgIn) {
  StaticJsonBuffer<200> jsonBuffer;
  char msg[301];
  strncpy(msg, msgIn.c_str(), 300);
  JsonArray& root = jsonBuffer.parseArray(msg);
  //[{"messageType":"42c3546a088b3ef8b8d3","sender":"IoT App","messages":[{"command":"switch on"}]}]

  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  //Serial.print(root[0].is<JsonObject&>());
  //Serial  << "type:" << root.get(0) << endl;
  if (root[0].is<JsonObject&>()) {
    Serial << " Received cmd: " << root[0]["messages"][0]["command"].asString() << endl;
    if (root[0]["messages"][0]["command"].asString()[0]) {
      processCommand(root[0]["messages"][0]["command"].asString());
    }
  } else {
    Serial << "no cmd" << endl;
  }

}

