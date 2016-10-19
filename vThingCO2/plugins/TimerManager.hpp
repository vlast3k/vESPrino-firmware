#ifndef TimerManager_h
#define TimerManager_h

#include "interfaces\Plugin.hpp"
#include <LinkedList.h>
#include <Timer.h>

#define TMR_START 1
#define TMR_STOPPED 0
#define PROP_TZOFFSET F("tz.offset");
#define ONE_DAY_MS 24L*60*60*1000
class TimerManagerClass : public Plugin {
public:
  Timer *registerTimer(Timer *t, int mode = TMR_START) {
    timers.add(t);
    if (mode == TMR_START) t->Start();
    return t;
  };
  void loop() {
    for (int i=0; i < timers.size(); i++) {
      timers.get(i)->Update();
    }
  }

  static enum TimeInPeriod {TP_UNDEFINED, TP_IN, TP_OUT};

  static TimeInPeriod isTimeInPeriod(const char *startc, const char* endc) {
    int32_t  now   = getGMTime();
    if (now < 0) return TP_UNDEFINED;
    now += PropertyList.readLongProperty(PROP_TZOFFSET) * 60 * 1000;
    if (now < 0) now += ONE_DAY_MS;
    uint32_t start = zuluToMs(startc);
    uint32_t end   = zuluToMs(endc);
    if (start > end) end += ONE_DAY_MS;
    if ( (now > start && now < end) ||
         (now + ONE_DAY_MS > start && now + ONE_DAY_MS < end)) return TP_IN;
    return TP_OUT;
  }

  static uint32_t zuluToMs(const char *d1) {
    return (((d1[17]-'0')*10 + d1[18]-'0')*60 + (d1[20]-'0')*10 + d1[21]-'0')*60*1000;
  }

  static void upldateLastTime() {
    const char* getTime() {
    HTTPClient http;
    const char *headers[1] = {"Date"};
    http.begin(F("http://google.com/"));
    http.collectHeaders(headers, 1);
    int rc = http.sendRequest("HEAD");
    const char *d1 = http.header("Date").c_str();
    rtcMemStore.setGenData(RTCMemStore::GEN_LASTTIME, zuluToMs(d1+17));
  }

  static int32_t getGMTime() {
    uint32_t lastTime  = rtcMemStore.getGenData(RTCMemStore::GEN_LASTTIME);
    uint32_t msCounter = rtcMemStore.getGenData(RTCMemStore::GEN_MSCOUNTER);
    if (lastTime == 0 || msCounter > 24L*60*60*1000) updateLastTime();
    return lastTime ? lastTime + msCounter : -1;
  }

  const char* getName() {
    return "TimerManager";
  }

private:
  LinkedList<Timer*> timers = LinkedList<Timer*>();

};

#endif
