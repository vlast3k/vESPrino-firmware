#include "plugins/PowerManager.hpp"
#include "MenuHandler.hpp"
#include <ESP8266WiFi.h>
#include "common.hpp"
#include <Timer.h>
#include "plugins/TimerManager.hpp"
extern TimerManagerClass TimerManager;

#define PROP_ITERATION_DURATION F("pwr.iterdur")
#define PROP_TIMEOUT_INTERVAL F("pwr.timeoutint")
uint8_t PowerManagerClass::IterationDurationS = 0;

PowerManagerClass::PowerManagerClass() {
  registerPlugin(this);
  isLowPower = false;
}
void PowerManagerClass::cmdDeepSleep(const char *line) {
  PowerManager.cmdDeepSleepInst(line);
}

void PowerManagerClass::cmdDeepSleepInst(const char *line) {
  int type = atoi(strchr(line, ' ') + 1);
//  LOGGER << "set deepsleep mode:" << type<< endl;
  rtcMemStore.setDeepSleep(true);
  switch (type) {
    case 0: ESP.deepSleep(1, WAKE_RF_DISABLED); break;
    case 1: ESP.deepSleep(1, WAKE_RF_DEFAULT); break;
    case 2: isLowPower = true; break;
  }
  delay(1000);
}
void PowerManagerClass::onProperty(String &key, String &value) {
  if (key == PROP_ITERATION_DURATION) IterationDurationS = atol(value.c_str());
  else if (key == PROP_TIMEOUT_INTERVAL) timeoutIntervalS = atol(value.c_str());
  else if (key == PROP_DEBUG) DEBUG = PropertyList.toBool(value);

  //Serial << "key = " << key << " is " << PROP_DEBUG << " : " << (key == PROP_DEBUG) << endl;
}

bool PowerManagerClass::setupInt(MenuHandler *handler) {
  WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
  //WiFi.setSleepMode(WIFI_NONE_SLEEP);
  //IterationDurationS = PropertyList.readLongProperty(PROP_ITERATION_DURATION);
  if (!IterationDurationS) IterationDurationS = 30;
  if (IterationDurationS < 15) {
    LOGGER << F("Timeout disabled since IterationDuration is: ") << IterationDurationS << endl;
    timeoutIntervalS = 0;
  } else if (timeoutIntervalS == -1) {
    timeoutIntervalS = 300;
  }
  timer = TimerManager.registerTimer(new Timer(1000L * timeoutIntervalS,
    PowerManagerClass::onTimeout), timeoutIntervalS ? TMR_START: TMR_STOPPED);
  handler->registerCommand(new MenuEntry(F("nop"), CMD_BEGIN, &PowerManagerClass::onNop, F("nop - no command, send to prevent going into power-safe operation during UI interaction")));
  handler->registerCommand(new MenuEntry(F("deepsleep"), CMD_BEGIN, PowerManagerClass::cmdDeepSleep, F("nop - no command, send to prevent going into power-safe operation during UI interaction")));
  isLowPower = rtcMemStore.wasInDeepSleep();
  wokeFromDeepSleep = isLowPower;
  if (isLowPower) {
    LOGGER << F("Device will go to Deep Sleep mode, once data is sent. Press [Enter] to abort\n");
  }
  return false;

  //isLowPower = rtcMemStore.getTest();
}

void PowerManagerClass::onTimeout() {
  PowerManager.onTimeoutInst();
}

void PowerManagerClass::onTimeoutInst() {
  timer->Stop();
  isLowPower = true;
  LOGGER << F("Switched to power-safe mode. Press key during start or restart device to exit.") << endl;
}

void PowerManagerClass::onNop(const char *line) {
  PowerManager.onNopInst(line);
}

void PowerManagerClass::onNopInst(const char *line) {
  int t = timeoutIntervalS;
  if (strchr(line, ' '))  t = atoi(strchr(line, ' ') + 1);
  //if (DEBUG) LOGGER << F("PowerManager timeout: ") << t << endl;
  timeoutIntervalS = t;
  if (t == 0) {
    timer->Stop();
    //if (DEBUG) LOGGER << F("PowerManager disabled")<< endl;
  } else {
    timer->setInterval(1000L*t);
    timer->Start();
  }
  isLowPower = false;
}

void PowerManagerClass::loopPowerManager() {
  //LOGGER << "Power Manager: " << millis() << endl;
  //LOGGER.flush();
  if (isLowPower) {
    uint32_t sec = IterationDurationS;
    //if (PropertyList.hasProperty(PROP_SND_INT)) sec = PropertyList.readLongProperty(PROP_SND_INT);
  //rtcMemStore.setIterations(rtcMemStore.getIterations() + 1);

    LOGGER << F("Completed in: ") << millis() << "ms\n";
    LOGGER << F("Going into power-safe mode for ") << sec << F(" seconds") << endl;
    rtcMemStore.setDeepSleep(true);
    //LOGGER << "is deep sleep1: " <<rtcMemStore.wasInDeepSleep();
    LOGGER.flush();
    //delay(100);
    //ESP.deepSleep(20L*1000*1000);
    LOGGER.flushLog();
    rtcMemStore.setGenData(GEN_MSCOUNTER, 100 + millis() + sec*1000 + rtcMemStore.getGenData(GEN_MSCOUNTER));
    ESP.deepSleep(sec*1000*1000);
    delay(2000);
  } else {
    delay(1);
  }
}

bool PowerManagerClass::isWokeFromDeepSleep() {
  return wokeFromDeepSleep;
}
