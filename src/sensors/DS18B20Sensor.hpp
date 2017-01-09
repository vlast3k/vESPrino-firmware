#ifndef DS18B20Sensor_h
#define DS18B20Sensor_h

#include "interfaces/Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include <DallasTemperature.h>

class DS18B20Sensor : public Sensor {
public:
  DS18B20Sensor();
  bool setup(MenuHandler *handler);
  void getData(LinkedList<Pair*> *data);
  const char* getName() {
    return "DS18B20";
  }


private:
  OneWire *oneWireP;
  DallasTemperature *sensorsP;

  uint8_t port;
  void printAddress(DeviceAddress deviceAddress);
  float getRawTemperature(int mode);
  bool hasSensorOnPort(const char *port);
  float getRawTemperature();
  void initPort(char *result);
};
#endif
