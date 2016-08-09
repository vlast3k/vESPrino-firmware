#ifndef Sensor_h
#define Sensor_h

#include "LinkedList.h"
#include "Pair.h"
#include "Plugin.hpp"

class Sensor : public Plugin {
public:
  virtual void getData(LinkedList<Pair*> *data);
  virtual const char* getSensorId();
  virtual float getValue();
};

#endif
