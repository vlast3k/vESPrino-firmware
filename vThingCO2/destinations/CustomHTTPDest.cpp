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
  handler->registerCommand(new MenuEntry(F("custom_url_jadd"), CMD_BEGIN, &CustomHTTPDest::menuAddCustomUrlJ, F("custom_url_add \"idx\",\"url\"")));
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
void CustomHTTPDest::menuAddCustomUrlJ(const char *line) {
  //Serial << "menuAddCustomUrl" << endl;
  char sidx[10];
  line = extractStringFromQuotes(line, sidx, sizeof(sidx));
  if (sidx[0] == 0) {
    Serial << F("Command not recognized");
    return;
  }
  int idx = atoi(sidx);
  PropertyList.putArrayProperty(F("custom_url_arr"), idx, line);
}

bool CustomHTTPDest::parseJSONUrl(String &s, String &url, String &method, String &ct, String &pay) {
  StaticJsonBuffer<400> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(s.c_str());
  if (!root.success()) {
    SERIAL_PORT << F("Parsing failed! : ") << s.c_str() << endl;
    return false;
  }
  if (root.containsKey("url"))    url    = root["url"].asString();
  if (root.containsKey("method")) method = root["method"].asString();
  if (root.containsKey("ct"))     ct     = root["ct"].asString();
  if (root.containsKey("pay"))    pay    = root["pay"].asString();
  return true;
}

void CustomHTTPDest::process(LinkedList<Pair *> &data) {
  Serial << F("CustomHTTPDest::process") << endl;
  int i=0;
  do {
    String s = PropertyList.getArrayProperty(F("custom_url_arr"), i++);
    String url, method = "GET", contentType = "", pay = "";
    if (!s.length()) return;
    if (s.charAt(0) == '#') s = s.substring(1);
    if (s.charAt(0) == '{') {
      if (!parseJSONUrl(s, url, method, contentType, pay)) continue;
    } else {
      url = s;
    }
    replaceValuesInURL(data, url);
    replaceValuesInURL(data, pay);
    invokeURL(url, method, contentType, pay);
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

void CustomHTTPDest::invokeURL(String &url, String &method, String &contentType, String &pay) {
  if (waitForWifi() != WL_CONNECTED) return;
  Serial << F("CustomHTTPDest::invoke = ") << url << endl;
  if (pay.length()) Serial << F("CustomHTTPDest::payload = ") << pay << endl;
  Serial.flush();
  HTTPClient http;
  http.begin(url);
  if (contentType.length()) http.addHeader(F("Content-Type"), contentType);
  AT_FW_Plugin::processResponseCodeATFW(&http, http.sendRequest(method.c_str(), pay));
  //addHCPIOTHeaders(&http, token);

}
