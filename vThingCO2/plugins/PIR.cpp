#include <Arduino.h>
#include "common.hpp"
void initPIR() {
    pinMode(D6, INPUT);
}
void handlePIR() {
  static bool hadMotion = false;
  if (hadMotion) {
    if (digitalRead(D6)) return;
    hadMotion = false;
    menuHandler.scheduleCommand("vecmd pirOff");
    return;
  }
  if (!hadMotion && digitalRead(D6)) {
    delay(100);
    if (digitalRead(D6)) { //to prevend the sporadi fires
      hadMotion=true;
      menuHandler.scheduleCommand("vecmd pirOn");
    }
  }
}
