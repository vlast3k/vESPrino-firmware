#ifndef PowerManager_h
#define PowerManager_h


#include "MenuHandler.hpp"
#include <Timer.h>


class PowerManagerClass  {
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
  const char* getName() {
    return "PowerManager";
  }
  const static uint8_t IterationDurationS = 15;
  void static cmdDeepSleep(const char *line) ;
  void cmdDeepSleepInst(const char *line) ;
  bool isLowPower;
private:
  Timer *timer;
  bool wokeFromDeepSleep;


};



#endif
