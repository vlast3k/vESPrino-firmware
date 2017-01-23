#include "interfaces/Destination.hpp"
#include "interfaces/Plugin.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "CustomHTTPDest.hpp"
#include "common.hpp"
#include "MenuHandler.hpp"
#include "plugins/PropertyList.hpp"
#include "plugins/AT_FW_Plugin.hpp"
#include "plugins/WifiStuff.hpp"
extern WifiStuffClass WifiStuff;

extern NeopixelVE neopixel; // there was a reason to put it here and not in commons

CustomHTTPDest::CustomHTTPDest() {
  registerDestination(this);
}

bool CustomHTTPDest::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("custom_url_add"), CMD_BEGIN, &CustomHTTPDest::menuAddCustomUrl, F("custom_url_add \"idx\",\"url\"")));
  handler->registerCommand(new MenuEntry(F("custom_url_jadd"), CMD_BEGIN, &CustomHTTPDest::menuAddCustomUrlJ, F("custom_url_add \"idx\",\"url\"")));
  handler->registerCommand(new MenuEntry(F("custom_url_clean"), CMD_EXACT, &CustomHTTPDest::menuCleanCustomUrl, F("custom_url_clean - clean all custom urls")));
  handler->registerCommand(new MenuEntry(F("call_url"), CMD_BEGIN, &CustomHTTPDest::cmdCallUrl, F("call_url http://xxx  or call_url {\"url\"=\"<url>\", \"method\"=\"POST|GET..\", \"ct\"=\"content-type\", \"pay\"=\"body payload\"}")));
  return false;
}

void CustomHTTPDest::menuCleanCustomUrl(const char *line) {
  PropertyList.removeArrayProperty(F("custom_url_arr"));
}

void CustomHTTPDest::menuAddCustomUrl(const char *line) {
  //LOGGER << "menuAddCustomUrl" << endl;
  char sidx[10], url[200];
  line = extractStringFromQuotes(line, sidx, sizeof(sidx));
  line = extractStringFromQuotes(line, url, sizeof(url));
  if (sidx[0] == 0 || url[0] == 0) {
    LOGGER << F("Command not recognized");
    return;
  }
  int idx = atoi(sidx);
  PropertyList.putArrayProperty(F("custom_url_arr"), idx, url);
}

void CustomHTTPDest::menuAddCustomUrlJ(const char *line) {
  char sidx[10];
  line = extractStringFromQuotes(line, sidx, sizeof(sidx));
  if (sidx[0] == 0) {
    LOGGER << F("Command not recognized");
    return;
  }
  int idx = atoi(sidx);
  PropertyList.putArrayProperty(F("custom_url_arr"), idx, line);
}

void CustomHTTPDest::cmdCallUrl(const char *line) {
  line = strchr(line, ' ');
  if (!line) return;
  String s = line+1;
  LinkedList<Pair *> values = LinkedList<Pair* >();
  customHTTPDest.invokeURL(s, values);
}

bool CustomHTTPDest::parseJSONUrl(String &s, String &url, String &method, String &ct, String &pay) {
  StaticJsonBuffer<400> jsonBuffer;
  JsonObject& root = jsonBuffer.parseObject(s.c_str());
  if (!root.success()) {
    LOGGER << F("Parsing failed! : ") << s.c_str() << endl;
    return false;
  }
  if (root.containsKey("url"))    url    = root["url"].asString();
  if (root.containsKey("method")) method = root["method"].asString();
  if (root.containsKey("ct"))     ct     = root["ct"].asString();
  if (root.containsKey("pay"))    pay    = root["pay"].asString();
  return true;
}

bool CustomHTTPDest::isOKResponse(int resp) {
  LOGGER << F("Response Code: ") << resp << endl;
  return resp >=200 && resp < 300;
}

bool CustomHTTPDest::process(LinkedList<Pair *> &data) {
  LOGGER << F("CustomHTTPDest::process") << endl;
  int i=0;
  bool status = true;
  do {
    String s = PropertyList.getArrayProperty(F("custom_url_arr"), i++);
    if (!s.length()) break;
    status = isOKResponse(invokeURL(s, data)) && status;
  } while(true);

  return (i==1) ? false : status;
  return status;
}

void CustomHTTPDest::replaceValuesInURL(LinkedList<Pair *> &data, String &s) {
  for (int i=0; i < data.size(); i++) {
    Pair *p = data.get(i);
    String skey = String("%") + p->key + String("%");
    s.replace(skey, String(p->value));
  }
}

int CustomHTTPDest::invokeURL(String &s, LinkedList<Pair *> &data) {
  if (!s.length()) return -11;
  String url, method = "GET", contentType = "", pay = "";
  if (s.charAt(0) == '#') s = s.substring(1);
  if (s.charAt(0) == '{') {
    if (!parseJSONUrl(s, url, method, contentType, pay)) return -12;
  } else {
    url = s;
  }
  replaceValuesInURL(data, url);
  replaceValuesInURL(data, pay);
  if (hasPlaceholders(url) || hasPlaceholders(pay)) return -13;
  return invokeURL(url, method, contentType, pay);
}

int CustomHTTPDest::invokeURL(String &url, String &method, String &contentType, String &pay) {
  if (WifiStuff.waitForWifi() != WL_CONNECTED) return -10;
  LOGGER << F("Calling HTTP: [") << url << "]" << endl;
  if (pay.length()) LOGGER << F("CustomHTTPDest::payload = ") << pay << endl;
  LOGGER.flush();
  HTTPClient http;
  http.begin(url);
  if (contentType.length()) http.addHeader(F("Content-Type"), contentType);
  return AT_FW_Plugin::processResponseCodeATFW(&http, http.sendRequest(method.c_str(), pay));
}
