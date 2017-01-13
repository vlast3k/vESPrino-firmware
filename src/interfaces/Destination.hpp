#ifndef Destination_h
#define Destination_h

#include <LinkedList.h>
#include "Pair.h"
#include "Plugin.hpp"

class Destination : public Plugin {
public:
  virtual bool process(LinkedList<Pair*> &data);
  bool hasPlaceholders(String &data) {
    if (data.indexOf("%TEMP%") > -1 ||
        data.indexOf("%HUM%") > -1 ||
        data.indexOf("%CO2%") > -1 ) return true ;
    return false;
  }
};

#endif
