#ifndef CustomHTTPDest_h
#define CustomHTTPDest_h

#include "interfaces\Destination.hpp"
#include "interfaces\Plugin.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"
#include "MenuHandler.hpp"

class CustomHTTPDest : public Destination {
public:
  CustomHTTPDest();
  void process(LinkedList<Pair*> &data);
  void processURL(String &s);
  void setup(MenuHandler *handler);
  void loop() {};
  static void menuAddCustomUrl(const char *line);
  static void menuCleanCustomUrl(const char *line);

};

#endif
