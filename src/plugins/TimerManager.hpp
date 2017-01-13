#ifndef TimerManager_h
#define TimerManager_h

#include "interfaces/Plugin.hpp"
#include <LinkedList.h>
#include <Timer.h>
#include <ESP8266HTTPClient.h>
#include <plugins/PropertyList.hpp>
#include <RTCMemStore.hpp>

#define TMR_START 1
#define TMR_STOPPED 0
#define PROP_TZOFFSET F("tz.offset")
#define ONE_DAY_MS 24L*60*60*1000
#define NO_TIME 0xFFFFFFFFL
enum TimeInPeriod {TP_UNDEFINED, TP_IN, TP_OUT};
class TimerManagerClass : public Plugin {
public:
  Timer *registerTimer(Timer *t, int mode = TMR_START);
  void loop();
  static TimeInPeriod isTimeInPeriod(const char *startc, const char* endc);
  static uint32_t timeToMs(const char *d1);
  static uint32_t updateLastTime();
  static uint32_t getGMTime();
  const char* getName() {
    return "TimerManager";
  }

private:
  LinkedList<Timer*> timers = LinkedList<Timer*>();

};

#endif
