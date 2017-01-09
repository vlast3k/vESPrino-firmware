#include <Wire.h>
#include "sensors/TSL2561Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "common.hpp"
#include "TSL2561.h"
#include <I2CHelper.hpp>

TSL2561Sensor::TSL2561Sensor() {
  registerSensor(this);
}

bool TSL2561Sensor::setup(MenuHandler *handler) {
  //handler->regsterCommand(new MenuEntry(F("tslInit"), CMD_EXACT, &TSL2561Sensor::onCmdInit, F("")));
  if (initSensor()) return true;
  closeSensor();
  return false;
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
  if (!rtcMemStore.hasSensor(RTC_SENSOR_TSL2561)) return false;
  if (I2CHelper::I2CHelper::i2cSDA ==  -1) return false;
  closeSensor();
  bool init = false;
  tsl = new TSL2561();
  for (int i=0; i < 5; i++) {
    init = tsl->begin();
    if (init) break;
    I2CHelper::I2CHelper::i2cHigh();
    delay(100);
  }
  if (!init) {
    //if (DEBUG) LOGGER << F("TSL2561 - init failed!") << endl;
    rtcMemStore.setSensorState(RTC_SENSOR_TSL2561, false);
    return false;
  }
  //LOGGER << F("Found TSL2561 - LUX Sensor") << endl;
  return true;
}

void TSL2561Sensor::closeSensor() {
  if (!tsl) return;
  delete tsl;
  tsl = NULL;
}
