#ifndef BlynkDest_h
#define BlynkDest_h
#define BLYNK_NO_FANCY_LOGO

#include "interfaces/Destination.hpp"
#include "interfaces/Plugin.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "MenuHandler.hpp"
#include "Arduino.h"

//#define PROP_CUSTOM_URL_ARR F("custom_url_arr")
class BlynkDest : public Destination {
public:
  BlynkDest();
  bool setup(MenuHandler *handler);
  void loop();

  bool process(LinkedList<Pair*> &data);

  Pair* getPair(LinkedList<Pair *> &data, String &key);

  const char* getName() {
    return "Blynk";
  }

private:
  bool enabled = false;
  bool calledConfig= false;
  char *auth;
  //static const __FlashStringHelper* PROP_CUSTOM_URL_ARR;// = F("custom_url_arr");
};

#endif
