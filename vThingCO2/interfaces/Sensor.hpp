#include "LinkedList.h"
#include "Pair.h"
#include "Plugin.hpp"

class Sensor : public Plugin {
public:
  virtual void getData(LinkedList<Pair*> *data);
};
