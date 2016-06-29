#ifdef VTHING_STARTER

void getDweetCommand(const char* dwFor, String &data1) {
    String url = "http://dweet.io/get/latest/dweet/for/";
    url += dwFor;
    SERIAL << "Getting dweets for " << url << endl;
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    if(httpCode > 0) {
        String payload = http.getString();
        
        SERIAL.println(payload.c_str());  
        char ttt[1000];
        strcpy(ttt, payload.c_str());        
        char rrr[] = "{\"this\":\"failed\",\"with\":404,\"because\":\"we couldn't find this\"}";
//        char rrr[] = "{\"this\":\"failed\",\"with\":404,\"because\":\"we couldnt find this\"}";
        //{"this":"succeeded","by":"getting","the":"dweets","with":[{"thing":"dwvltest","created":"2016-06-29T03:58:12.055Z","content":{"data1":"VLAUE2","data2":""}}]}
        StaticJsonBuffer<400> jsonBuffer;
        JsonObject& root = jsonBuffer.parseObject(payload);
        if (!root.success()) {
          SERIAL << "Parsing failed!" << endl;
          return;
        }
        SERIAL << "received" << endl;
        root.printTo(SERIAL);
        SERIAL << endl;
        SERIAL << root.containsKey("this") << endl; 
        //SERIAL << root["this"] << endl; 
       // SERIAL << root["this"].asString() << endl; 
    //SERIAL << root[0]["messages"].is<JsonArray&>() << endl;
    //SERIAL << root[0]["messages"][0].is<JsonObject&>() << endl;
    //SERIAL << root[0]["messages"][0].asObject().containsKey("color") << endl;
//        if (!strcmp(root["this"].asString(), "succeeded")) {
//          SERIAL << "received command" << root["with"][0]["content"]["data1"].asString() << "-" << endl;
//        }
    } else {
      SERIAL << F("Failed to getDweet from: ") << url << ", due to: " <<http.errorToString(httpCode) << endl;
    }
}
void onGetDweets() {
  String res;
  getDweetCommand("vESPrino", res);
}


#endif
