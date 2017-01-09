#ifndef TSL2561Sensor_h
#define TSL2561Sensor_h

#include "interfaces/Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "TSL2561.h"

class TSL2561Sensor : public Sensor {
public:
  TSL2561Sensor();
  bool setup(MenuHandler *handler);
  void getData(LinkedList<Pair*> *data);
  const char* getName() {
    return "TSL2561";
  }
  bool initSensor();
  void closeSensor();
private:
  static void onCmdInit(const char *ignore);
  TSL2561 *tsl = NULL;


};
#endif
