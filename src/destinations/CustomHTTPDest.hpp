#ifndef CustomHTTPDest_h
#define CustomHTTPDest_h

#include "interfaces/Destination.hpp"
#include "interfaces/Plugin.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "MenuHandler.hpp"
#include "Arduino.h"

#define PROP_ONGESTURE F("onGesture")
class CustomHTTPDest : public Destination {
public:
  CustomHTTPDest();
  bool setup(MenuHandler *handler);
  void loop() {};
  static void menuAddCustomUrl(const char *line);
  static void menuAddCustomUrlJ(const char *line);
  static void menuCleanCustomUrl(const char *line);
  static void cmdCallUrl(const char *line);
  bool process(LinkedList<Pair*> &data);
  void replaceValuesInURL(LinkedList<Pair *> &data, String &s);
  int invokeURL(String &url, LinkedList<Pair *> &data);
  int invokeURL(String &url, String &method, String &contentType, String &pay);
  bool parseJSONUrl(String &s, String &url, String &method, String &ct, String &pay);
  bool isOKResponse(int resp);
  const char* getName() {
    return "CustomHTTP";
  }

private:
  //static const __FlashStringHelper* PROP_CUSTOM_URL_ARR;// = F("custom_url_arr");
};

#endif
