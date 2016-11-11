#ifndef Sensor_h
#define Sensor_h

#include "LinkedList.h"
#include "Pair.h"
#include "Plugin.hpp"

class Sensor : public Plugin {
public:
  virtual void getData(LinkedList<Pair*> *data);
  bool hasSensor = false;
  // virtual const char* getSensorId() { return "";};
  // virtual float getValue() {return 0;};
};

#endif
