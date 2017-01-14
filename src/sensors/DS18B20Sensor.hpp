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

  static void scanBus(const char *ignore);
  static void readTemperatures(const char *ignore);
  void scanBusInst();
  void readTemperaturesInst(LinkedList<float> *list = NULL);
  double getAverageTemperature(LinkedList<float> *list);
  static void computeTempAdjustments(const char *ignore);
  void computeTempAdjustmentsInst();
  bool readAdjustedTemperatures(LinkedList<float> *list);
  void appendTemperatures(LinkedList<Pair *> *data);

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
