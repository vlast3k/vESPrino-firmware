#ifndef BH1750Sensor_h
#define BH1750Sensor_h

#include "interfaces/Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "BH1750FVI.h"

class BH1750Sensor : public Sensor {
public:
  BH1750Sensor();
  bool setup(MenuHandler *handler);
  void getData(LinkedList<Pair*> *data);
  const char* getName() {
    return "BH1750";
  }
  bool initSensor();
  void closeSensor();
  void onCmdTestInst();
private:
  static void onCmdTest(const char *ignore);
  BH1750FVI *tsl = NULL;
   void measureMT(uint8_t mt);
   void measure();


};
#endif
