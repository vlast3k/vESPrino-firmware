#ifndef CO2Sensor_h
#define CO2Sensor_h
#include "interfaces\Sensor.hpp"
#include "interfaces\Plugin.hpp"

class CO2Sensor : public Sensor {
public:
  void setup(MenuHandler *handler);
  void loop();
  const char* getSensorId();
  float getValue();
};

#endif
