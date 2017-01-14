#include "plugins/TimerManager.hpp"
#include "interfaces/Plugin.hpp"
#include <LinkedList.h>
#include <Timer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266WiFi.h>
#include <plugins/PropertyList.hpp>
#include <Streaming.h>
#include "plugins/LoggingPrinter.hpp"
#include "plugins/WifiStuff.hpp"

extern WifiStuffClass WifiStuff;

extern LoggingPrinter LOGGER;
extern boolean DEBUG;
wl_status_t waitForWifi(uint16_t timeoutMs);

extern RTCMemStore rtcMemStore;
  Timer *TimerManagerClass::registerTimer(Timer *t, int mode) {
    timers.add(t);
    if (mode == TMR_START) t->Start();
    return t;
  }

  void TimerManagerClass::loop() {
    for (int i=0; i < timers.size(); i++) {
      timers.get(i)->Update();
    }
  }

  const char *msToStr(uint32_t ms, String &s) {
    if (ms == 0) return "nan";
    ms /= 1000;
    ms /= 60;
    s = "";
    s+= (int)ms/60;
    s+= ":";
    s+= ms %60;
    //Serial << "mstostr: " << s << endl;
    return s.c_str();
  }

  TimeInPeriod TimerManagerClass::isTimeInPeriod(const char *startc, const char* endc) {
    int32_t  now   = getGMTime();
    String s;
    if (now == NO_TIME) return TP_UNDEFINED;
    if (DEBUG) LOGGER << F("now(): ") << msToStr(now, s) << endl;
    now += PropertyList.readLongProperty(PROP_TZOFFSET) * 60 * 1000;
    if (DEBUG) LOGGER << F("now(tz adjusted): ") << msToStr(now, s) << endl;
    if (now < 0) now += ONE_DAY_MS;
    uint32_t start = timeToMs(startc);
    uint32_t end   = timeToMs(endc);
    if (start > end) end += ONE_DAY_MS;
    if ( (now > start && now < end) ||
         (now + ONE_DAY_MS > start && now + ONE_DAY_MS < end)) return TP_IN;
    return TP_OUT;
  }

  uint32_t TimerManagerClass::timeToMs(const char *d1) {
    //converts 19:40 to ms from 00:00
    return (((d1[0]-'0')*10 + d1[1]-'0')*60 + (d1[3]-'0')*10 + d1[4]-'0')*60*1000;
  }

  void getTimeFromGoogle(String &time) {
    if (DEBUG) LOGGER << F("Check time from google") << endl;
    if (WifiStuff.waitForWifi(7000) != WL_CONNECTED) return;
    HTTPClient http;
    const char *headers[1] = {"Date"};
    http.begin(F("http://google.com/"));
    http.collectHeaders(headers, 1);
    int rc = http.sendRequest("HEAD");
    if (rc < 0) return;
    time = http.header("Date");
    //const char *d1 = http.header("Date").c_str();

  }

  uint32_t TimerManagerClass::updateLastTime() {
    String googleTime;
    String s;
    getTimeFromGoogle(googleTime);
    const char *d1 = googleTime.c_str();
    if (strlen(d1) < 20) return NO_TIME;
    //if (DEBUG) LOGGER << F("Time From Google: ") << d1 << endl;
    uint32_t tg = timeToMs(d1 + strlen(d1) - 12);
    rtcMemStore.setGenData(GEN_LASTTIME, tg);
    rtcMemStore.setGenData(GEN_TIME_LAST_UPD_INT, rtcMemStore.getGenData(GEN_MSCOUNTER) + millis());
    //LOGGER << "  time google: " << msToStr(tg, s) << endl;
    //LOGGER << "  time google: " << msToStr(tg, s) << endl;
    return tg;
  }

  uint32_t TimerManagerClass::getGMTime() {
    //LOGGER << "GetGMTTime..." << endl;
    //String s;
    uint32_t lastTime  = rtcMemStore.getGenData(GEN_LASTTIME);
    uint32_t msCounter = rtcMemStore.getGenData(GEN_MSCOUNTER) + millis();
    uint32_t lastUpdInt= rtcMemStore.getGenData(GEN_TIME_LAST_UPD_INT);
    //LOGGER << "  lastTime: " << msToStr(lastTime, s) << " @ " << lastTime << endl;
    //LOGGER << "  msCounter: " << msToStr(msCounter, s) << " @ " << msCounter << endl;
    //LOGGER.flush();
    if (lastTime == 0 || msCounter - lastUpdInt > 60L*60*1000) {
      uint32_t x = updateLastTime();
      if (x != NO_TIME) lastTime = x;
    }
    msCounter = rtcMemStore.getGenData(GEN_MSCOUNTER) + millis();
    lastUpdInt= rtcMemStore.getGenData(GEN_TIME_LAST_UPD_INT);
    return lastTime ? lastTime + msCounter - lastUpdInt : NO_TIME;
  }
