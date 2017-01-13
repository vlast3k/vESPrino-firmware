
#ifndef PowerManager_h
#define PowerManager_h

#include "interfaces/Plugin.hpp"
#include "MenuHandler.hpp"
#include <Timer.h>


class PowerManagerClass : public Plugin   {
public:
  PowerManagerClass();
  void onProperty(String &key, String &value);
  bool setupInt(MenuHandler *handler);

  void loopPowerManager();
  static void onTimeout();
  static void onNop(const char* line);
  void onTimeoutInst();
  void onNopInst(const char* line);
  const char* getName() {
    return "PowerManager";
  }
  static uint8_t IterationDurationS;
  void static cmdDeepSleep(const char *line) ;
  void cmdDeepSleepInst(const char *line) ;
  bool isWokeFromDeepSleep();
  bool isLowPower;
private:
  Timer *timer;
  bool wokeFromDeepSleep;
  int16_t timeoutIntervalS = -1;


};



#endif
