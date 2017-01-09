#ifndef BMP085Sensor_h
#define BMP085Sensor_h

#include "interfaces/Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "Adafruit_BMP085.h"

//#define SEALEVELPRESSURE_HPA (1013.25)

class BMP085Sensor : public Sensor {
public:
  BMP085Sensor();
  bool setup(MenuHandler *handler);
  void getData(LinkedList<Pair*> *data);
  const char* getName() {
    return "BMP085";
  }
  bool initSensor();
  void closeSensor();
private:
  static void onCmdInit(const char *ignore);
  Adafruit_BMP085 *bme = NULL;


};
#endif
