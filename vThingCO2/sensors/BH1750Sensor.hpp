#ifndef BH1750Sensor_h
#define BH1750Sensor_h

#include "interfaces\Sensor.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"
#include <BH1750FVI.h>

class BH1750Sensor : public Sensor {
public:
  BH1750Sensor();
  void setup(MenuHandler *handler);
  void getData(LinkedList<Pair*> *data);
  const char* getName() {
    return "BH1750";
  }
  bool initSensor();
  void closeSensor();
private:
  static void onCmdInit(const char *ignore);
  BH1750FVI *tsl = NULL;


};
#endif
