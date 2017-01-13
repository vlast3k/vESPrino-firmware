//#define BLYNK_DEBUG // Optional, this enables lots of prints
#include "common.hpp"
#define BLYNK_PRINT LOGGER
#include <BlynkSimpleEsp8266.h>

#include "interfaces/Destination.hpp"
#include "interfaces/Plugin.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "BlynkDest.hpp"
#include "MenuHandler.hpp"
#include "plugins/PropertyList.hpp"
#include "plugins/AT_FW_Plugin.hpp"
#include "plugins/WifiStuff.hpp"
extern WifiStuffClass WifiStuff;

#define PROP_BLYNK_ENABLED F("blynk.enabled")
#define PROP_BLYNK_AUTH F("blynk.auth")
#define PROP_BLYNK_CFG F("blynk.cfg")
// char auth[] = "77e6e715b5714bd797ff86489d863b76"; // Put your token here
//        Blynk.run();
BlynkDest::BlynkDest() {
  //enabled = true;
  registerDestination(this);
}

bool BlynkDest::setup(MenuHandler *handler) {
  //handler->registerCommand(new MenuEntry(F("serial_dump_toggle"), CMD_EXACT, &BlynkDest::toggle, F("serial_dump_toggle toggle serial dump output")));
  enabled = PropertyList.readBoolProperty(PROP_BLYNK_ENABLED);
  return enabled;
  // if (enabled) {
  //   menuHandler.scheduleCommand("nop 0");
  // }
}

void BlynkDest::loop() {
  if (!enabled) return;
  if (WifiStuff.waitForWifi() != WL_CONNECTED) return;
  if (!calledConfig) {
    _blynkWifiClient = new WiFiClient();
    _blynkTransport = new BlynkArduinoClient(*_blynkWifiClient);
    Blynk = new BlynkWifi(*_blynkTransport);
    auth = new char[40];
    strcpy(auth, PropertyList.readProperty(PROP_BLYNK_AUTH));
    Blynk->config(auth);
    calledConfig = true;
  }
  Blynk->run();
}

Pair* BlynkDest::getPair(LinkedList<Pair *> &data, String &key) {
  for (int i=0; i < data.size(); i++) {
    Pair *p = data.get(i);
    if (key == p->key) return p;
  }
  return NULL;
}

bool BlynkDest::process(LinkedList<Pair *> &data) {
  if (!enabled) return true;
  loop();
  String cfg = PropertyList.readProperty(PROP_BLYNK_CFG);
  if (DEBUG)  LOGGER << F("[BLYNK] cfg: ") << cfg << endl;
  const char *str = cfg.c_str();
  char buf[10];
  int i = 0;
  int count = getListItemCount(str);

  if ((count % 2) != 0) {
    LOGGER << F("Bad Blynk configuration : ") << str << endl;
    return false;
  }

  for (int i=0; i< 50 && !Blynk->connected(); i++) {
    delay(10);
    Blynk->run();
  }

  for (int i=0; i < count; i+=2) {
    String vPort = getListItem(str, buf, i);
    String key   = getListItem(str, buf, i+1);
    Pair *p = getPair(data, key);
    if (!p) continue;
    if (DEBUG)  LOGGER << F("[BLYNK] ") << vPort << F("=") << key << F(" : ") << p->value << endl;
    Blynk->virtualWrite(atoi(vPort.c_str()+1), atof(p->value.c_str()));
  }
  return true;
}
