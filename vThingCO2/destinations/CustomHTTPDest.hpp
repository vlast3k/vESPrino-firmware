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
  void setup(MenuHandler *handler);
  void loop();
  static void menuAddCustomUrl(const char *line);
};

#endif
