#ifndef PowerManager_h
#define PowerManager_h

#include "interfaces\Plugin.hpp"
#include "MenuHandler.hpp"
#include <Timer.h>


class PowerManagerClass : public Plugin {
public:
  PowerManagerClass() {
    isLowPower = false;
  };
  void setup(MenuHandler *handler);
  void loopPowerManager();
  static void onTimeout();
  static void onNop(const char* ignore);
  void onTimeoutInst();
  void onNopInst();

private:
  Timer *timer;
  bool isLowPower;

};



#endif
