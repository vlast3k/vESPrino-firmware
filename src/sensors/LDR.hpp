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
  //void toggleInst();

private:
  //bool enabled = false;
};
#endif
