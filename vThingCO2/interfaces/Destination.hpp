#ifndef Destination_h
#define Destination_h

#include <LinkedList.h>
#include "Pair.h"
#include "Plugin.hpp"

class Destination : public Plugin {
public:
  virtual void process(LinkedList<Pair*> &data);
};

#endif
