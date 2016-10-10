#include "plugins\PowerManager.hpp"
#include "MenuHandler.hpp"
#include <ESP8266WiFi.h>
#include "common.hpp"
#include <Timer.h>

void PowerManagerClass::cmdDeepSleep(const char *line) {
  PowerManager.cmdDeepSleepInst(line);
}
void PowerManagerClass::cmdDeepSleepInst(const char *line) {
  int type = atoi(strchr(line, ' ') + 1);
  Serial << "set deepsleep mode:" << type<< endl;
  rtcMemStore.setDeepSleep(true);
  switch (type) {
    case 0: ESP.deepSleep(1, WAKE_RF_DISABLED); break;
    case 1: ESP.deepSleep(1, WAKE_RF_DEFAULT); break;
    case 2: isLowPower = true; break;
  }
  delay(1000);
}
void PowerManagerClass::setup(MenuHandler *handler) {
  //WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  timeoutIntervalS = 180;
  timer = TimerManager.registerTimer(new Timer(1000L * timeoutIntervalS, PowerManagerClass::onTimeout, millis));
  handler->registerCommand(new MenuEntry(F("nop"), CMD_BEGIN, &PowerManagerClass::onNop, F("nop - no command, send to prevent going into power-safe operation during UI interaction")));
  handler->registerCommand(new MenuEntry(F("deepsleep"), CMD_BEGIN, PowerManagerClass::cmdDeepSleep, F("nop - no command, send to prevent going into power-safe operation during UI interaction")));
  isLowPower = rtcMemStore.wasInDeepSleep();
  wokeFromDeepSleep = isLowPower;
  if (isLowPower) {
    Serial << F("Device will go to Deep Sleep mode, once data is sent. Press [Enter] to abort\n");
  }
  //isLowPower = rtcMemStore.getTest();
}

void PowerManagerClass::onTimeout() {
  PowerManager.onTimeoutInst();
}

void PowerManagerClass::onTimeoutInst() {
  timer->Stop();
  isLowPower = true;
  Serial << F("Switched to power-safe mode. Press key during start or restart device to exit.") << endl;
}

void PowerManagerClass::onNop(const char *line) {
  PowerManager.onNopInst(line);
}

void PowerManagerClass::onNopInst(const char *line) {
  int t = timeoutIntervalS;
  if (strchr(line, ' '))  t = atoi(strchr(line, ' ') + 1);
  if (DEBUG) Serial << F("PowerManager timeout: ") << t << endl;
  timeoutIntervalS = t;
  if (t == 0) {
    timer->Stop();
    if (DEBUG) Serial << F("PowerManager disabled")<< endl;
  } else {
    timer->setInterval(1000L*t);
    timer->Start();
  }
  isLowPower = false;
}

void PowerManagerClass::loopPowerManager() {
  //Serial << "Power Manager: " << millis() << endl;
  //Serial.flush();
  if (isLowPower) {
    uint32_t sec = IterationDurationS;
    //if (PropertyList.hasProperty(PROP_SND_INT)) sec = PropertyList.readLongProperty(PROP_SND_INT);
  //rtcMemStore.setIterations(rtcMemStore.getIterations() + 1);

    Serial << F("Going into power-safe mode for ") << sec << F(" seconds") << endl;
    rtcMemStore.setDeepSleep(true);
    //Serial << "is deep sleep1: " <<rtcMemStore.wasInDeepSleep();
    Serial.flush();
    //delay(100);
    //ESP.deepSleep(20L*1000*1000);
    ESP.deepSleep(sec*1000*1000);
    delay(2000);
  } else {
    delay(1);
  }
}

bool PowerManagerClass::isWokeFromDeepSleep() {
  return wokeFromDeepSleep;
}
