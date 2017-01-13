#ifndef CO2Sensor_h
#define CO2Sensor_h
#include "interfaces/Sensor.hpp"
#include "interfaces/Plugin.hpp"
#include "interfaces/Pair.h"
#include "Timer.h"
#include "CubicGasSensors.h"
#include "common.hpp"

#define PROP_NO_CUBIC_CO2 F("cubic.co2.missing")
#define PROP_CUBIC_CO2_POWERSAFE F("cubic.co2.powersafe")

class CO2Sensor : public Sensor {
public:
  CO2Sensor();
  bool setup(MenuHandler *handler);
  void loop() {};
  const char* getSensorId();
  float getValue();
  void getData(LinkedList<Pair*> *data);
  const char* getName() {
    return "CM110x";
  }

private:
  void onCo2Status(CubicStatus status);
  void onStopLED();
  void resetCO2();
  static void onCo2Status_static(CubicStatus status);
  static void onStopLED_static();
  static void resetCO2_static(const char *ignore);
  CubicGasSensors cubicCo2;
  boolean startedCO2Monitoring = false;
  //RunningAverage raCO2Raw(4);
  Thr co2Threshold;//("CO2", 1);
  Timer *tmrStopLED;
};

#endif
