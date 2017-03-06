#ifndef TGS8100_h
#define TGS8100_h

#include "interfaces/Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"

class TGS8100 : public Sensor {
public:
  TGS8100();
  bool setup(MenuHandler *handler);
  void getData(LinkedList<Pair*> *data);
  int readSensorValue(uint16_t &raw, uint16_t &rs, double &ppm, uint16_t &rsa, uint16_t &r0);
  static void test(const char *ignore);
  const char* getName() {
    return "TGS8100";
  }
  // static void toggle(const char *line);
  // void toggleInst();

private:
  bool enabled = false;
};
#endif
