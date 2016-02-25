int BTTN_PIN = D3;
void ex2_attachInterrupt(void (*)());

bool shouldSend = false;
void onButton() {
  Serial <<"bttn" << endl;
  if (digitalRead(BTTN_PIN) == 0) shouldSend = true;
}

void attachButton() {
  pinMode(BTTN_PIN, INPUT_PULLUP);
  attachInterrupt(BTTN_PIN, onButton, CHANGE);
}



int clicks = 5;
void doSend() {
  if (shouldSend == false) return;
  shouldSend = false;
  Serial << "Sending button" << endl;
  HTTPClient http;
  //https://iotmmsi024148trial.hanatrial.ondemand.com/com.sap.iotservices.mms/v1/api/http/push/e46304a8-a410-4979-82f6-ca3da7e43df9
  //{"method":"http", "sender":"My IoT application", "messageType":"42c3546a088b3ef8b8d3", "messages":[{"command":"yellow"}]}
  String rq = String("https://") + mmsHost + "/com.sap.iotservices.mms/v1/api/http/push/" + deviceId;
  Serial << "Sending: " << rq << endl;                
  http.begin(rq);
  http.addHeader("Content-Type",  "application/json;charset=UTF-8");
  http.setAuthorization("P1940433103", "Abcd1234");
  //http.addHeader("Authorization", String("Bearer ") + authToken);  
  String post = "";
  post += "{\"method\":\"http\", \"sender\":\"My IoT application\", \"messageType\":\"42c3546a088b3ef8b8d3\", \"messages\":[{\"command\":\"" + colors[(clicks++) % COLOR_COUNT] + "\"}]}";
  int httpCode = http.POST(post);
  // httpCode will be negative on error
  if(httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      String payload = http.getString();
      Serial.println(payload);          
  } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
}

