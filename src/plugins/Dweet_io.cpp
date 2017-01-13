#include "plugins/Dweet_io.hpp"
#include "MenuHandler.hpp"
#include <Timer.h>
#include "plugins/TimerManager.hpp"
extern TimerManagerClass TimerManager;
#include "plugins/WifiStuff.hpp"
extern WifiStuffClass WifiStuff;

#define PROP_DWEET_CMDKEY F("dweet.cmdkey")
#define PROP_DWEET_AUTOSTART F("dweet.autostart")
#define PROP_DWEET_ACCEPT_STORED_DWEETS F("dweet.acceptStoredDweets")

DweetIOClass::DweetIOClass() {
  registerPlugin(this);
}

bool DweetIOClass::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("dweet_start"), CMD_BEGIN, DweetIOClass::cmdDweetStart, F("dweet_start interval_Sec")));
  handler->registerCommand(new MenuEntry(F("dweet_process"), CMD_EXACT, DweetIOClass::cmdDweetProcess, F("dweet_process - checks if there are new dweets")));
  String s = PropertyList.readProperty(PROP_DWEET_AUTOSTART);
  if (s.length()) {
    s = String("dweet_start ") + s;
    menuHandler.scheduleCommand(s.c_str());
    //cmdDweetStart(s.c_str());
  }
  isAcceptStoredDweets = PropertyList.readBoolProperty(PROP_DWEET_ACCEPT_STORED_DWEETS);
  return false;

}

void DweetIOClass::cmdDweetStart(const char *cmd) {
  DweetIO.cmdDweetStartInst(cmd);
}

void DweetIOClass::cmdDweetProcess(const char *cmd) {
  DweetIO.cmdDweetProcessInst(cmd);
}

void DweetIOClass::cmdDweetProcessInst(const char *cmd) {
  char line[1000];
  if (!getDweetCommand(line)) return;
  menuHandler.scheduleCommand(line);
}

void DweetIOClass::cmdDweetStartInst(const char *cmd) {
  int intervalSec = 5;
  if (strchr(cmd, ' ')) intervalSec = atoi(strchr(cmd, ' ') + 1);
  if (intervalSec > 0 && intervalSec < 5) intervalSec = 5;
  dwKey = PropertyList.readProperty(PROP_DWEET_CMDKEY);
  if (dwKey.length() == 0) dwKey = String(F("vThing_")) + String(ESP.getChipId(), HEX);
  LOGGER << F("Reading dweets from: ") << dwKey << F(" each ") << intervalSec << F("sec") << endl;
  LOGGER.flush();
  LOGGER << F("Send commands via: http://dweet.io/dweet/for/") << dwKey << F("?cmd=...") << endl;
  LOGGER.flush();
  if (timer == NULL) timer = TimerManager.registerTimer(new Timer(intervalSec*1000, onGetDweets));
  if (intervalSec == 0) timer->Stop();
  else timer->setInterval(intervalSec*1000);
  //menuHandler.scheduleCommand("nop 0");
  //onGetDweets();
}

void DweetIOClass::onGetDweets() {
  menuHandler.scheduleCommand(F("dweet_process"));
}

bool DweetIOClass::getDweetCommand(char *cmd) {
    if (WifiStuff.waitForWifi() != WL_CONNECTED) return false;
    uint32_t mstart = millis();
    char lastDweet[30];
    rtcMemStore.getLastDweet(lastDweet);

    if (!dwKey.length()) return false;
    if (WifiStuff.waitForWifi() != WL_CONNECTED) return false ;
    String url = String(F("http://dweet.io/get/latest/dweet/for/")) + dwKey;

    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();
    if(httpCode <= 0) {
      LOGGER << F("Failed to getDweet from: ") << url << F(", due to: ") << http.errorToString(httpCode) << endl;
      return false;
    }

    String payload = http.getString();
    char pay2[1000];
    strcpy(pay2, payload.c_str());
    StaticJsonBuffer<400> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(pay2);
    if (!root.success()) {
      LOGGER << F("Parsing failed! : ") << payload.c_str() << endl;
      return false;
    }
    if (DEBUG) LOGGER << F("Got Dweets for: ") << millis() - mstart << F(" ms\n");
    if (!root.containsKey("this") || strcmp(root["this"].asString(), "succeeded")) return false;
    if (strcmp(lastDweet, root["with"][0]["created"].asString()) == 0) return false;
    if (!*lastDweet && !isAcceptStoredDweets) {
      rtcMemStore.setLastDweet(root["with"][0]["created"].asString());
      return false;
    } else {
      rtcMemStore.setLastDweet(root["with"][0]["created"].asString());
      strcpy(cmd, root["with"][0]["content"]["cmd"].asString());
      if (DEBUG) LOGGER << F("New Dweet: ") << cmd << endl;
      return true;
    }
}
