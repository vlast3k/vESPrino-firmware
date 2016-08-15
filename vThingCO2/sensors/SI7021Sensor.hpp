#ifndef SI7021Sensor_h
#define SI7021Sensor_h

#include <SI7021.h>
#include "interfaces\Sensor.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"

class SI7021Sensor : public Sensor {
public:
  SI7021Sensor();
  //void setup(MenuHandler *handler);
  void getData(LinkedList<Pair*> *data);
private:
  SI7021 *si7021;
  bool initSensor();
  void closeSensor();

};
#endif
