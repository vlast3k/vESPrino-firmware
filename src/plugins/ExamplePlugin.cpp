#include "plugins/ExamplePlugin.hpp"
#include "MenuHandler.hpp"
#include "plugins/GPIO.hpp"
#include <Timer.h>

extern ExamplePluginClass ExamplePlugin;

//#define PROP_ANNEM_CONNECTED F("annem.connected")

ExamplePluginClass::ExamplePluginClass() {
  registerPlugin(this);
}

bool ExamplePluginClass::setup(MenuHandler *handler) {
   //handler->registerCommand(new MenuEntry(F("annemStart"), CMD_BEGIN, ExamplePluginClass::cmdStart, F("annemStart D7 - port number start ExamplePlugin")));
  //  String s = PropertyList.readProperty(PROP_ANNEM_CONNECTED);
  //  if (s.length()) {
  //    s = String("annemStart ") + s;
  //    menuHandler.scheduleCommand(s.c_str());
  //  }
   return false;
}

void ExamplePluginClass::cmdStart(const char *line) {
  ExamplePlugin.cmdStartInst(line);
}

void ExamplePluginClass::cmdStartInst(const char *line) {
}

void ExamplePluginClass::loop() {
}
