#ifndef BME280Sensor_h
#define BME280Sensor_h

#include "interfaces/Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "Adafruit_BME280.h"

#define SEALEVELPRESSURE_HPA (1013.25)

class BME280Sensor : public Sensor {
public:
  BME280Sensor();
  bool setup(MenuHandler *handler);
  void getData(LinkedList<Pair*> *data);
  const char* getName() {
    return "BME280";
  }
  bool initSensor();
  void closeSensor();
private:
  static void onCmdInit(const char *ignore);
  Adafruit_BME280 *bme = NULL;


};
#endif
