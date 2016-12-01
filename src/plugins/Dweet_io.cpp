#include "plugins/Dweet_io.hpp"
#include "MenuHandler.hpp"
#include <Timer.h>



Timer * DweetIOClass::timer = NULL;
#define PROP_DWEET_CMDKEY F("dweet.cmdkey")
#define PROP_DWEET_AUTOSTART F("dweet.autostart")

DweetIOClass::DweetIOClass() {
  registerPlugin(this);
}

void DweetIOClass::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("dweet_start"), CMD_BEGIN, DweetIOClass::cmdDweetStart, F("dweet_start interval_Sec")));
  String s = PropertyList.readProperty(PROP_DWEET_AUTOSTART);
  if (s.length()) {
    s = String("dweet_start ") + s;
    menuHandler.scheduleCommand(s.c_str());
    //cmdDweetStart(s.c_str());
  }
}

void DweetIOClass::cmdDweetStart(const char *cmd) {
  int intervalSec = 5;
  if (strchr(cmd, ' ')) intervalSec = atoi(strchr(cmd, ' ') + 1);
  if (intervalSec < 5) intervalSec = 5;
  Serial << F("Reading dweets from: ") << PropertyList.readProperty(PROP_DWEET_CMDKEY) << F(" each ") << intervalSec << F("sec") << endl;
  Serial.flush();
  Serial << F("Send commands via: http://dweet.io/dweet/for/") << PropertyList.readProperty(PROP_DWEET_CMDKEY) << F("?cmd=...") << endl;
  Serial.flush();
  if (DweetIOClass::timer == NULL) DweetIOClass::timer = TimerManager.registerTimer(new Timer(intervalSec*1000, onGetDweets));
  if (intervalSec == 0) DweetIOClass::timer->Stop();
  else DweetIOClass::timer->setInterval(intervalSec*1000);
  menuHandler.scheduleCommand("nop 0");
  //onGetDweets();
}

void DweetIOClass::onGetDweets() {
  char line[1000];
  if (!getDweetCommand(line)) return;
  menuHandler.scheduleCommand(line);
}

bool DweetIOClass::getDweetCommand(char *cmd) {
    if (waitForWifi() != WL_CONNECTED) return false;
    char lastDweet[30];
    rtcMemStore.getLastDweet(lastDweet);
    uint32_t mstart = millis();

    if (!PropertyList.hasProperty(PROP_DWEET_CMDKEY)) return false;
    if (waitForWifi() != WL_CONNECTED) return false ;
    String url = String(F("http://dweet.io/get/latest/dweet/for/")) + PropertyList.readProperty(PROP_DWEET_CMDKEY);

    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    if(httpCode <= 0) {
      SERIAL_PORT << F("Failed to getDweet from: ") << url << F(", due to: ") << http.errorToString(httpCode) << endl;
      return false;
    }

    String payload = http.getString();
    char pay2[1000];
    strcpy(pay2, payload.c_str());
    StaticJsonBuffer<400> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(pay2);
    if (!root.success()) {
      SERIAL_PORT << F("Parsing failed! : ") << payload.c_str() << endl;
      return false;
    }
    if (DEBUG) Serial << F("Got Dweets for: ") << millis() - mstart << F(" ms\n");
    if (!root.containsKey("this") || strcmp(root["this"].asString(), "succeeded")) return false;
    if (strcmp(lastDweet, root["with"][0]["created"].asString()) == 0) return false;
    rtcMemStore.setLastDweet(root["with"][0]["created"].asString());
    //strcpy(lastDweet, root["with"][0]["created"].asString());
    strcpy(cmd, root["with"][0]["content"]["cmd"].asString());
    if (DEBUG) SERIAL_PORT << F("New Dweet: ") << cmd << endl;
    return true;
}
