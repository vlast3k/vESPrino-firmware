#include <Wire.h>
#include "sensors/TSL2561Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "common.hpp"
#include "lib/TSL2561.h"

TSL2561Sensor::TSL2561Sensor() {
  registerSensor(this);
}

void TSL2561Sensor::setup(MenuHandler *handler) {
  //handler->regsterCommand(new MenuEntry(F("tslInit"), CMD_EXACT, &TSL2561Sensor::onCmdInit, F("")));
  initSensor();
  closeSensor();
}

void TSL2561Sensor::onCmdInit(const char *ignore) {
  //tsl2561Sensor.initSensor();
}

void TSL2561Sensor::getData(LinkedList<Pair *> *data) {
  if (!initSensor()) return;
  data->add(new Pair("LUX", String(tsl->getLuxAutoScale())));
  closeSensor();
}

bool TSL2561Sensor::initSensor() {
  if (i2cSDA == -1) return false;
  closeSensor();
  bool init = false;
  tsl = new TSL2561();
  for (int i=0; i < 5; i++) {
    init = tsl->begin();
    if (init) break;
    i2cHigh();
    delay(100);
  }
  if (!init) {
    if (DEBUG) Serial << F("TSL2561 - init failed!") << endl;
    return false;
  }
  SERIAL_PORT << F("Found TSL2561 - LUX Sensor") << endl;
  return true;
}

void TSL2561Sensor::closeSensor() {
  if (!tsl) return;
  delete tsl;
  tsl = NULL;
}
