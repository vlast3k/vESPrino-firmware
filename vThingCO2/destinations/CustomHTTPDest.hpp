#ifndef CustomHTTPDest_h
#define CustomHTTPDest_h

#include "interfaces\Destination.hpp"
#include "interfaces\Plugin.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"

class CustomHTTPDest : public Destination {
public:
  void process(LinkedList<Pair*> &data);
  void setup();
  void loop();
};

#endif
