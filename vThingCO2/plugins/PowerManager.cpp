#include "plugins\PowerManager.hpp"
#include "MenuHandler.hpp"
#include <ESP8266WiFi.h>
#include "common.hpp"
#include <Timer.h>


void PowerManagerClass::setup(MenuHandler *handler) {
  WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
  //updateDeepSleepWake();
  timer = TimerManager.registerTimer(new Timer(20000L, PowerManagerClass::onTimeout));
  handler->registerCommand(new MenuEntry(F("nop"), CMD_EXACT, &PowerManagerClass::onNop, F("nop - no command, send to prevent going into power-safe operation during UI interaction")));
  isLowPower = deepSleepWake;
}

void PowerManagerClass::onTimeout() {
  PowerManager.onTimeoutInst();
}

void PowerManagerClass::onTimeoutInst() {
  timer->Stop();
  isLowPower = true;
  Serial << F("Switched to power-safe mode. Restart device to exit.") << endl;

}

void PowerManagerClass::onNop(const char *ignore) {
  PowerManager.onNopInst();
}
void PowerManagerClass::onNopInst() {
  timer->Start();
}

void PowerManagerClass::loopPowerManager() {
  Serial << "Power Manager: " << millis() << endl;
  Serial.flush();
  if (isLowPower) {
    Serial << F("Going into power-safe mode for 20 seconds") << endl;
    Serial.flush();
    delay(100);
    ESP.deepSleep(20L*1000*1000);
    delay(2000);
  } else {
    delay(500);
  }
}
