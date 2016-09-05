#ifndef CustomHTTPDest_h
#define CustomHTTPDest_h

#include "interfaces\Destination.hpp"
#include "interfaces\Plugin.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"
#include "MenuHandler.hpp"
#include "Arduino.h"

//#define PROP_CUSTOM_URL_ARR F("custom_url_arr")
class CustomHTTPDest : public Destination {
public:
  CustomHTTPDest();
  void setup(MenuHandler *handler);
  void loop() {};
  static void menuAddCustomUrl(const char *line);
  static void menuCleanCustomUrl(const char *line);
  void process(LinkedList<Pair*> &data);
  void replaceValuesInURL(LinkedList<Pair *> &data, String &s);
  void invokeURL(String &url);
  const char* getName() {
    return "CustomHTTP";
  }

private:
  //static const __FlashStringHelper* PROP_CUSTOM_URL_ARR;// = F("custom_url_arr");
};

#endif
