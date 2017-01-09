#ifndef APDS9960Sensor_h
#define APDS9960Sensor_h

#include "interfaces/Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "SparkFun_APDS9960.h"

class APDS9960Sensor : public Sensor {
public:
  APDS9960Sensor();
  bool setup(MenuHandler *handler);
  void getData(LinkedList<Pair*> *data);
  const char* getName() {
    return "APDS9960";
  }
  void loop();
  bool initSensor();
  void closeSensor();
  //static int isr_flag;
private:
  static void onCmdInit(const char *ignore);
  SparkFun_APDS9960 *sensor = NULL;

  //static void interruptRoutine();
  void handleGesture();



};
#endif
