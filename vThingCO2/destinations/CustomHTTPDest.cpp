#include "interfaces\Destination.hpp"
#include "interfaces\Plugin.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"
#include "CustomHTTPDest.hpp"
#include "common.hpp"
#include "MenuHandler.hpp"
#include "plugins\PropertyList.hpp"




CustomHTTPDest::CustomHTTPDest() {
  registerDestination(this);
//  registerPlugin(this);

}

void CustomHTTPDest::process(LinkedList<Pair *> &data) {
  int i=1;
  do {
    String s = PropertyList.getArrayProperty(F("custom_url"), i++);
    if (!s.length()) return;
    processURL(s);
  } while(true);
}

void CustomHTTPDest::processURL(String &s) {
}


void CustomHTTPDest::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("custom_url_add"), CMD_BEGIN, &CustomHTTPDest::menuAddCustomUrl, F("custom_url_add \"idx\",\"url\"")));
  handler->registerCommand(new MenuEntry(F("custom_url_clean"), CMD_EXACT, &CustomHTTPDest::menuCleanCustomUrl, F("custom_url_clean - clean all custom urls")));
}


void CustomHTTPDest::menuCleanCustomUrl(const char *line) {
  PropertyList.removeArrayProperty(F("custom_url"));
}

void CustomHTTPDest::menuAddCustomUrl(const char *line) {
  char sidx[10], url[200];
  line = extractStringFromQuotes(line, sidx, sizeof(sidx));
  line = extractStringFromQuotes(line, url, sizeof(url));
  if (sidx[0] == 0 || url[0] == 0) {
    Serial << F("Command not recognized");
    return;
  }
  int idx = atoi(sidx);
  PropertyList.putArrayProperty(F("custom_url"), idx, url);
}
