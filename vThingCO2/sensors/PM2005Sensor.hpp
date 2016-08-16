#ifndef PM2005Sensor_h
#define PM2005Sensor_h
#include "interfaces\Sensor.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"

class PM2005Sensor : public Sensor {
public:
  PM2005Sensor();
  void setup(MenuHandler *handler);
  void getData(LinkedList<Pair*> *data);
  char* getName() {
    return "PM2005";
  }
  // bool initSensor();
  // void closeSensor();
private:
//  static void onCmdInit(const char *ignore);
  bool intReadData(int &pm25, int &pm10, bool debug = false);
  bool intBegin(int sda = 0, int sca = 0);
};
#endif
