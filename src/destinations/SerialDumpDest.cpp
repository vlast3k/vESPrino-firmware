#include "interfaces/Destination.hpp"
#include "interfaces/Plugin.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "SerialDumpDest.hpp"
#include "common.hpp"
#include "MenuHandler.hpp"
#include "plugins/PropertyList.hpp"
#include "plugins/AT_FW_Plugin.hpp"

SerialDumpDest::SerialDumpDest() {
  enabled = true;
  registerDestination(this);
}

void SerialDumpDest::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("serial_dump_toggle"), CMD_EXACT, &SerialDumpDest::toggle, F("serial_dump_toggle toggle serial dump output")));
  //handler->registerCommand(new MenuEntry(F("ser"), CMD_EXACT, &SerialDumpDest::toggle, F("serial_dump_toggle toggle serial dump output")));
}

void SerialDumpDest::toggle(const char *line) {
  serialDumpDest.toggleInst();
}

void SerialDumpDest::toggleInst() {
  enabled != enabled;
  Serial << F("Serial Dump Destination:") << (enabled ? F("ENABLED") : F("DISABLED")) << endl;
}

void SerialDumpDest::process(LinkedList<Pair *> &data) {
  if (!enabled) return;
  Serial << F("SerialDumpDest::process") << endl;
  for (int i=0; i < data.size(); i++) {
    Pair *p = data.get(i);
    Serial << p->key << "\t: " << p->value << endl;
    Serial.flush();
  }
}
