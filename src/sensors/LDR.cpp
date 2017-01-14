#include <Wire.h>
#include "sensors/LDR.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "common.hpp"
#include "plugins/NeopixelVE.hpp"

extern NeopixelVE neopixel;

LDR::LDR() {
  //enabled = true;
  //registerSensor(this);
}

bool LDR::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("ldr"), CMD_EXACT, &LDR::test, F("LDR toggle testSesnor")));
  //enabled = PropertyList.readBoolProperty(F("test.sensor"));
  return true;

}

void LDR::test(const char* ignore) {
  for (int i=0; i < 1; i++) {
    int s0 = neopixel.getAmbientLight(0);
    delay(500);
    int s10  = neopixel.getAmbientLight(10);
    delay(500);
    int s50  = neopixel.getAmbientLight(50);
    delay(500);
    int s100  = neopixel.getAmbientLight(100);
    delay(500);
    int s200  = neopixel.getAmbientLight(200);
    delay(500);
    int s300  = neopixel.getAmbientLight(300);
    delay(200);
  //  LOGGER << "LDR: 0:" << s0 << ", s10: " << s10 << ", s50: "<< s50 << ", s100: " << s100 << ", s200: " << s200<< ", s300: " << s300 <<endl;
  }
}

void LDR::getData(LinkedList<Pair *> *data) {
  // LOGGER << F("LDR::getData") << endl;
  // int light = getLight();
  // data->add(new Pair("LIGHT", String(light)));
}
