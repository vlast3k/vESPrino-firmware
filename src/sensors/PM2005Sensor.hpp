#ifndef PM2005Sensor_h
#define PM2005Sensor_h
#include "interfaces/Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"

#define PROP_PM2005_QSTART F("pm2005.qstart")
#define PROP_PM2005_QEND   F("pm2005.qend")
#define PROP_PM2005_INT_ACT F("pm2005.int.active")
#define PROP_PM2005_INT_QUIET F("pm2005.int.quiet")

class PM2005Sensor : public Sensor {
public:
  PM2005Sensor();
  bool setup(MenuHandler *handler);
  void getData(LinkedList<Pair*> *data);
  const char* getName() {
    return "PM2005";
  }
  // bool initSensor();
  // void closeSensor();
private:
//  static void onCmdInit(const char *ignore);
  bool intReadData(int &pm25, int &pm10, int &status, int &mode) ;
  bool intBegin();
  void setDynamicMode();
  void checkMode(LinkedList<Pair *> *data = NULL);
  static void onCmdQuiet(const char *line);
  static void onCmdInterval(const char *line);
  void setTimingMeasuringMode(uint16_t intervalSec);
  void sendCommand(uint8_t *toSend);
};
#endif
