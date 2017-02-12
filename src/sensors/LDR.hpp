#ifndef LDR_h
#define LDR_h

#include "interfaces/Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"

class LDR : public Sensor {
public:
  LDR();
  bool setup(MenuHandler *handler);
  void getData(LinkedList<Pair*> *data);
  const char* getName() {
    return "LDR";
  }
  static void test(const char *line);
  static void testCont(const char *line);
  //void toggleInst();

private:
  // int lightOff = 0;
  // int lightOn = 0;
  //bool enabled = false;
};
#endif
