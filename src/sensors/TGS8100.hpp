#ifndef TGS8100_h
#define TGS8100_h

#include "interfaces/Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#define TGS8100_I2C_ADDR 0x8

class TGS8100 : public Sensor {
public:
  TGS8100();
  bool setup(MenuHandler *handler);
  void getData(LinkedList<Pair*> *data);
  static void test(const char *ignore);
  static void reset(const char *ignore);
  const char* getName() {
    return "TGS8100";
  }
  // static void toggle(const char *line);
  // void toggleInst();

private:
  uint16_t cMaxR0 = 0;
  uint16_t cMaxR0prv = 0;
  double cRsAdj = 0;
  bool enabled = false;
  uint32_t sensorStarted = 0;
  uint32_t iterationStarted = 0;

  bool sensorActive();
  void onIteration(uint32_t iterations);
  void processData(uint16_t value, uint16_t vcc, uint16_t &rs, double &ppm);
  double getHumAdj(float from, float to);
  double getTempAdj(float from, float to);
  void onProperty(String &key, String &value);
  int readSensorValue(uint16_t &raw, uint16_t &rs, double &ppm, uint16_t &vcc);
  uint8_t getCrc(uint8_t *data, int len);
  uint16_t getMaxR0();
};
#endif
