#include "plugins\PowerManager.hpp"
#include "MenuHandler.hpp"
#include <ESP8266WiFi.h>
#include "common.hpp"
#include <Timer.h>

void cmdDeepSleep(const char *line) {
  int type = atoi(strchr(line, ' ') + 1);
  Serial << "set deepsleep mode:" << type<< endl;
  switch (type) {
    case 0: ESP.deepSleep(1, WAKE_RF_DISABLED);
    case 1: ESP.deepSleep(1, WAKE_RF_DEFAULT);
  }
  delay(1000);
}
void PowerManagerClass::setup(MenuHandler *handler) {
  WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
  timer = TimerManager.registerTimer(new Timer(80000L, PowerManagerClass::onTimeout, millis));
  handler->registerCommand(new MenuEntry(F("nop"), CMD_EXACT, &PowerManagerClass::onNop, F("nop - no command, send to prevent going into power-safe operation during UI interaction")));
  handler->registerCommand(new MenuEntry(F("deepsleep"), CMD_BEGIN, cmdDeepSleep, F("nop - no command, send to prevent going into power-safe operation during UI interaction")));
  isLowPower = rtcMemStore.getDataExisted();
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

void PowerManagerClass::onNop(const char *ignore) {
  PowerManager.onNopInst();
}

void PowerManagerClass::onNopInst() {
  timer->Start();
  isLowPower = false;
}

void PowerManagerClass::loopPowerManager() {
  //Serial << "Power Manager: " << millis() << endl;
  //Serial.flush();
  if (isLowPower) {
    Serial << F("Going into power-safe mode for 20 seconds") << endl;
    Serial.flush();
    delay(100);
    ESP.deepSleep(20L*1000*1000);
    delay(2000);
  } else {
    delay(1);
  }
}
