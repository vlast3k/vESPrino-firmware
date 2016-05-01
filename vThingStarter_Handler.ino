#ifdef VTHING_STARTER
void initVThingStarter() {
  strip = new NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> (1, D4);
  Serial << "aaaaaaaaa" << endl;
  if (strip) {
    strip->Begin();
    strip->SetPixelColor(0, RgbColor(220, 220,0));
    strip->Show();  
  }    
  si7021init();
  dumpTemp();
  tmrTempRead = new Timer(15000L,    onTempRead);
  tmrCheckPushMsg = new Timer(1000L, handleSAP_IOT_PushService);
  tmrTempRead->Start();
  tmrCheckPushMsg->Start();
  attachButton();
}

void loopVThingStarter() {
      //heap("");
    tmrTempRead->Update();
    //SERIAL << "\n\n\n------ before push service\n\n\n";
    tmrCheckPushMsg->Update();
    //handleSAP_IOT_PushService();
    //SERIAL << "\n\n\n------ before do Send\n\n\n";
    doSend();
//    SERIAL << "\n\n\n------ before delay 5 sec\n\n\n";
    //SERIAL << ".";
    delay(1000);
}

#include <Si7021.h>
SI7021 *si7021;
void si7021init() {
    si7021 = new SI7021();
    si7021->begin(D1, D6); // Runs : Wire.begin() + reset()
    si7021->setHumidityRes(8); // Humidity = 12-bit / Temperature = 14-bit
}
void onTempRead() {
  float tmp = si7021->readTemp();
  float hum = si7021->readHumidity();
  SERIAL << F("Humidity : ") << hum << " %\t";
  SERIAL << F("Temp : "    ) << tmp << " C" << endl;
  
  String s = String("sndiot ") + tmp;
  sndIOT(s.c_str());
}

void dumpTemp() {
  SERIAL << F("Temp : ")     << si7021->readTemp() << " C" << endl;  
}

int BTTN_PIN = D3;
void ex2_attachInterrupt(void (*)());


void onButton() {
  if (digitalRead(BTTN_PIN) == 0) shouldSend = true;
}

void attachButton() {
  //pinMode(BTTN_PIN, INPUT_PULLUP);
  attachInterrupt(BTTN_PIN, onButton, CHANGE);
}

int clicks = 5;
void doSend() {
  if (shouldSend == false) return;
  shouldSend = false;    
  SERIAL << F("Button Clicked!") << endl;
  if(WiFi.status() != WL_CONNECTED) {
    SERIAL << F("Will not send: No WiFi") << endl;
    return;
  }
  if (!getJSONConfig(SAP_IOT_HOST)) {
    SERIAL << F("Will not send: No configuration") << endl;
    return;
  }
  HTTPClient http;
  //https://iotmmsi024148trial.hanatrial.ondemand.com/com.sap.iotservices.mms/v1/api/http/push/e46304a8-a410-4979-82f6-ca3da7e43df9
  //{"method":"http", "sender":"My IoT application", "messageType":"42c3546a088b3ef8b8d3", "messages":[{"command":"yellow"}]}
  String rq = String("https://") + getJSONConfig(SAP_IOT_HOST) + F("/com.sap.iotservices.mms/v1/api/http/data/") + getJSONConfig(SAP_IOT_DEVID) + "/" + getJSONConfig(SAP_IOT_BTN_MSGID) + "/sync?button=IA==";
  SERIAL << "Sending: " << rq << endl;                
  http.begin(rq);
  http.addHeader("Content-Type",  "application/json;charset=UTF-8");
  //http.setAuthorization("P1940433103", "Abcd1234");
  http.addHeader("Authorization", String("Bearer ") + getJSONConfig(SAP_IOT_TOKEN));  
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


String mmsHost ;//= "iotmmsi024148trial.hanatrial.ondemand.com";
String deviceId; //= "e46304a8-a410-4979-82f6-ca3da7e43df9";
String authToken;// = "8f337a8e54bd352f28c2892743c94b3";
//String colors[] = {"red","pink","lila","violet","blue","mblue","cyan","green","yellow","orange"};
#define COLOR_COUNT 10

//h iotmmsi024148trial.hanatrial.ondemand.com
//d c5c73d69-6a19-4c7d-9da3-b32198ba71f9
//m 2023a0e66f76d20f47d7
//v co2
//t 46de4fc404221b32054a8405f602fd


//uint32_t lastSAPCheck = -1000000L;
void handleSAP_IOT_PushService() {
    if(WiFi.status() != WL_CONNECTED) return;
    heap("");
    //Serial << " before get json config" << endl;
    if (!getJSONConfig(SAP_IOT_HOST)) return;
    //Serial << " after " << endl;
    String url = String("https://") + getJSONConfig(SAP_IOT_HOST) + F("/com.sap.iotservices.mms/v1/api/http/data/") + getJSONConfig(SAP_IOT_DEVID) ; 
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
    http.addHeader("Authorization", String("Bearer ") + getJSONConfig(SAP_IOT_TOKEN));  
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
    if (DEBUG) SERIAL << F("parseObject() failed: ") << msgIn << endl;
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
      SERIAL << F("command not recognized") << endl;
     }
  } 
//  else {
//    SERIAL << "no cmd" << endl;
//  }

}

#endif
