#ifndef SI7021Sensor_h
#define SI7021Sensor_h

#include <SI7021.h>
#include "interfaces/Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"

class SI7021Sensor : public Sensor {
public:
  SI7021Sensor();
  bool setup(MenuHandler *handler);
  void getData(LinkedList<Pair*> *data);
  const char* getName() {
    return "SI7021";
  }
  bool initSensor();
  void closeSensor();
private:
  //static void onCmdInit(const char *ignore);
  SI7021 *si7021 = NULL;
  float adj = 0;


};
#endif
