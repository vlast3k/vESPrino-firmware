#ifndef CDM7160Sensor_h
#define CDM7160Sensor_h
#include "interfaces/Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include <I2CHelper.hpp>

class CDM7160Sensor : public Sensor {
public:
  CDM7160Sensor();
  bool setup(MenuHandler *handler);
  void getData(LinkedList<Pair*> *data);
  const char* getName() {
    return "CDM7160";
  }
  // bool initSensor();
  // void closeSensor();
private:
  void configureSensor();

  static void onCmdTest(const char *ignore);
  static void onChangeReg(const char *ignore);
  static void onCmdLoop(const char *ignore);
  static void onPerf(const char *ignore);
  static void onCdmSwitch(const char *ignore);
  

  static int readCO2Raw();
  static int readCO2AutoRecover();
  static uint8_t readI2CByte(int addr);
  static void writeCfg(uint8_t reg, uint8_t bit, bool value);
  static int writeByte(uint8_t reg, uint8_t value);
  static void endChangeMode();
  static void startChangeMode() ;
  static bool readI2CBytes(int start, uint8_t *buf, int len);
  static bool readI2CBytesBrzo(int start, uint8_t *buf, int len);
  static const uint8_t CDM_ADDR_WRITE = 0x69;
  static const uint8_t CDM_ADDR_READ  = 0xE9;
  static const uint8_t CDM_FAILED_BYTE = 0xFF;
  static const uint8_t CDM_CTL_REG = 0x1;
  static const uint8_t CDM_ST1_REG = 0x2;
  static const uint8_t CDM_AVG_REG = 0x7;
  static const uint8_t CDM_FMODE = 0x4;
  static const uint8_t CDM_ST_BUSY = 0x80;
  static const uint8_t CDM_ST_AVGCOMPLTE = 0x10;
  static const uint8_t CDM_AVG_DEFAULT = 1;
  // bool intReadData(int &pm25, int &pm10, bool debug = false);
  // bool intBegin(int sda = 0, int sca = 0);
};
#endif
