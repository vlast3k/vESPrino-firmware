#include "interfaces\Destination.hpp"
#include "interfaces\Plugin.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"
#include "CustomHTTPDest.hpp"
#include "common.hpp"
#include "MenuHandler.hpp"
#include "plugins\PropertyList.hpp"
#include "plugins\AT_FW_Plugin.hpp"

CustomHTTPDest::CustomHTTPDest() {
  registerDestination(this);
}

void CustomHTTPDest::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("custom_url_add"), CMD_BEGIN, &CustomHTTPDest::menuAddCustomUrl, F("custom_url_add \"idx\",\"url\"")));
  handler->registerCommand(new MenuEntry(F("custom_url_clean"), CMD_EXACT, &CustomHTTPDest::menuCleanCustomUrl, F("custom_url_clean - clean all custom urls")));
}

void CustomHTTPDest::menuCleanCustomUrl(const char *line) {
  PropertyList.removeArrayProperty(F("custom_url_arr"));
}

void CustomHTTPDest::menuAddCustomUrl(const char *line) {
  //Serial << "menuAddCustomUrl" << endl;
  char sidx[10], url[200];
  line = extractStringFromQuotes(line, sidx, sizeof(sidx));
  line = extractStringFromQuotes(line, url, sizeof(url));
  if (sidx[0] == 0 || url[0] == 0) {
    Serial << F("Command not recognized");
    return;
  }
  int idx = atoi(sidx);
  PropertyList.putArrayProperty(F("custom_url_arr"), idx, url);
}

void CustomHTTPDest::process(LinkedList<Pair *> &data) {
  Serial << F("CustomHTTPDest::process") << endl;
  int i=0;
  do {
    String s = PropertyList.getArrayProperty(F("custom_url_arr"), i++);
    if (!s.length()) return;
    replaceValuesInURL(data, s);
    invokeURL(s);
  } while(true);
}

void CustomHTTPDest::replaceValuesInURL(LinkedList<Pair *> &data, String &s) {
  //Serial << "CustomHTTPDest::replaceUrl = " << s << endl;
  // for (int i=0; i < data.size(); i++) {
  //   Serial << data.get(i)->key << " = " << data.get(i)->value << "."<<endl;
  // }
  for (int i=0; i < data.size(); i++) {
    Pair *p = data.get(i);
    String skey = String("%") + p->key + String("%");
    s.replace(skey, String(p->value));
    //Serial << "after replace: key << " << skey << "." << p->value<< "." << String(p->value) << " " << s << endl;
  }
}

void CustomHTTPDest::invokeURL(String &url) {
  if (waitForWifi(1000) != WL_CONNECTED) return;
  String x = url;
  if (url.startsWith("#")) x = url.substring(1);
  Serial << F("CustomHTTPDest::invoke = ") << x << endl;
  Serial.flush();
  delay(100);
//  waitForWifi(1000);
  HTTPClient http;
  http.begin(x);
  //addHCPIOTHeaders(&http, token);
  int rc = AT_FW_Plugin::processResponseCodeATFW(&http, http.GET());
}
