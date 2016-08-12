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
  Serial <<"Custom http dest.setup" << endl;
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
  Serial << "CustomHTTPDest::process" << endl;
  int i=1;
  do {
    String s = PropertyList.getArrayProperty(F("custom_url_arr"), i++);
    if (!s.length()) return;
    replaceValuesInURL(data, s);
    invokeURL(s);
  } while(true);
}

void CustomHTTPDest::replaceValuesInURL(LinkedList<Pair *> &data, String &s) {
  Serial << "CustomHTTPDest::replaceUrl = " << s << endl;
  for (int i=0; i < data.size(); i++) {
    Pair *p = data.get(i);
    String skey = String("%") + p->key + String("%");
    s.replace(skey, String(p->value));
  }
}

void CustomHTTPDest::invokeURL(String &url) {
  Serial << "CustomHTTPDest::invoke = " << url << endl;
  HTTPClient http;
  http.begin(url);
  //addHCPIOTHeaders(&http, token);
  int rc = AT_FW_Plugin::processResponseCodeATFW(&http, http.GET());
}