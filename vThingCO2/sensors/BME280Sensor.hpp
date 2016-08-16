#ifndef BME280Sensor_h
#define BME280Sensor_h

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "interfaces\Sensor.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"

#define SEALEVELPRESSURE_HPA (1013.25)

class BME280Sensor : public Sensor {
public:
  BME280Sensor();
  void setup(MenuHandler *handler);
  void getData(LinkedList<Pair*> *data);
  char* getName() {
    return "BME280";
  }
  bool initSensor();
  void closeSensor();
private:
  static void onCmdInit(const char *ignore);
  Adafruit_BME280 *bme;


};
#endif
