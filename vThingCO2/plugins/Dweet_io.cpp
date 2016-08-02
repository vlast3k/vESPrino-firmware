#ifdef VTHING_STARTER
#include "common.hpp"

void onGetDweets() {
  char line[200];
  boolean res = getDweetCommand(line);
  if (!res) return;
  handleDWCommand(line);
}

boolean getDweetCommand(char *cmd) {
    static char lastDweet[30];

    char tmp[200];
    if (!getJSONConfig("vespDWCmd", tmp)[0]) return false;
    String url = String("http://dweet.io/get/latest/dweet/for/") + getJSONConfig("vespDWCmd", tmp);

    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    if(httpCode <= 0) {
      SERIAL << F("Failed to getDweet from: ") << url << ", due to: " << http.errorToString(httpCode) << endl;
      return false;
    }

    String payload = http.getString();
    StaticJsonBuffer<400> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(payload.c_str());
    if (!root.success()) {
      SERIAL << F("Parsing failed! : ") << payload.c_str() << endl;
      return false;
    }
    if (!root.containsKey("this") || strcmp(root["this"].asString(), "succeeded")) return false;
    if (strcmp(lastDweet, root["with"][0]["created"].asString()) == 0) return false;
    strcpy(lastDweet, root["with"][0]["created"].asString());
    strcpy(cmd, root["with"][0]["content"]["cmd"].asString());
    SERIAL << "New Dweet: " << cmd << endl;
    return true;
}

#endif
