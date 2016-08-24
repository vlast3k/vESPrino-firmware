#ifndef CDM7160Sensor_h
#define CDM7160Sensor_h
#include "interfaces\Sensor.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"

class CDM7160Sensor : public Sensor {
public:
  CDM7160Sensor();
  void setup(MenuHandler *handler);
  void getData(LinkedList<Pair*> *data);
  char* getName() {
    return "CDM7160";
  }
  // bool initSensor();
  // void closeSensor();
private:
  static void onCmdTest(const char *ignore);
  static void onChangeMode(const char *ignore);
  static int readCO2(bool debug);
  static uint8_t readI2CByte(int addr);
  static void writeCfg(uint8_t reg, uint8_t bit, bool value);
  static void writeByte(uint8_t reg, uint8_t value);
  static void endChangeMode();
  static void startChangeMode() ;
  // bool intReadData(int &pm25, int &pm10, bool debug = false);
  // bool intBegin(int sda = 0, int sca = 0);
};
#endif
