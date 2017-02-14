#ifndef RGBLedDest_h
#define RGBLedDest_h

#include "interfaces/Destination.hpp"
#include "interfaces/Plugin.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "MenuHandler.hpp"
#include "Arduino.h"

//#define PROP_CUSTOM_URL_ARR F("custom_url_arr")
class RGBLedDest : public Destination {
public:
  RGBLedDest();
  bool setup(MenuHandler *handler);
  void loop() {};
  void onProperty(String &key, String &value);

  bool process(LinkedList<Pair*> &data);
  static void test(const char *line);
  void testInst();
  void mapColor(int current, int minVal, int maxVal);
  const char* getName() {
    return "RGBLed";
  }

private:
  bool enabled;
  String cfgKey;
  int16_t vmin, vmax;
  //static const __FlashStringHelper* PROP_CUSTOM_URL_ARR;// = F("custom_url_arr");
};

#endif
