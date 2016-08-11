#include "interfaces\Destination.hpp"
#include "interfaces\Plugin.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"
#include "CustomHTTPDest.hpp"
#include "common.hpp"
#include "MenuHandler.hpp"

CustomHTTPDest customHTTPDest;


CustomHTTPDest::CustomHTTPDest() {
  registerDestination(this);
//  registerPlugin(this);

}

void CustomHTTPDest::process(LinkedList<Pair *> &data) {

}

void CustomHTTPDest::setup(MenuHandler *handler) {

  handler->registerCommand(new MenuEntry(F("cust_http"), CMD_BEGIN, &CustomHTTPDest::menuAddCustomUrl, F("cust_http \"idx\",\"url\"")));

}

void CustomHTTPDest::loop() {

}

void CustomHTTPDest::menuAddCustomUrl(const char *line) {

}
