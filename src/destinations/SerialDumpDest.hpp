#ifndef SerialDumpDest_h
#define SerialDumpDest_h

#include "interfaces/Destination.hpp"
#include "interfaces/Plugin.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "MenuHandler.hpp"
#include "Arduino.h"

//#define PROP_CUSTOM_URL_ARR F("custom_url_arr")
class SerialDumpDest : public Destination {
public:
  SerialDumpDest();
  bool setup(MenuHandler *handler);
  void loop() {};

  bool process(LinkedList<Pair*> &data);
  static void toggle(const char *line);
  void toggleInst();

  const char* getName() {
    return "SerialDump";
  }

private:
  bool enabled;
  //static const __FlashStringHelper* PROP_CUSTOM_URL_ARR;// = F("custom_url_arr");
};

#endif
