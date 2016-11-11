
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
  static void onNop(const char* line);
  void onTimeoutInst();
  void onNopInst(const char* line);
  const char* getName() {
    return "PowerManager";
  }
  const static uint8_t IterationDurationS = 30;
  void static cmdDeepSleep(const char *line) ;
  void cmdDeepSleepInst(const char *line) ;
  bool isWokeFromDeepSleep();
  bool isLowPower;
private:
  Timer *timer;
  bool wokeFromDeepSleep;
  uint8_t timeoutIntervalS;


};



#endif
