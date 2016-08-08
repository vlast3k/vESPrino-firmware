#include "LinkedList.h"
#include "Pair.h"
#include "Plugin.hpp"

class Sender : public Plugin {
public:
  virtual void process(LinkedList<Pair*> *data);
};
