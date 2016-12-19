#ifndef DS18B20Sensor_h
#define DS18B20Sensor_h

#include "interfaces/Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include <DallasTemperature.h>

class DS18B20Sensor : public Sensor {
public:
  DS18B20Sensor();
  void setup(MenuHandler *handler);
  void getData(LinkedList<Pair*> *data);
  const char* getName() {
    return "DS18B20";
  }


private:
  bool enabled = false;
  void printAddress(DeviceAddress deviceAddress);
  float getRawTemperature(int mode);
};
#endif
