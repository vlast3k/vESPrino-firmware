#include <Wire.h>
#include "sensors\BH1750Sensor.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"
#include "common.hpp"
#include <BH1750FVI.h>

BH1750Sensor::BH1750Sensor() {
  registerSensor(this);
}

void BH1750Sensor::setup(MenuHandler *handler) {
  //handler->regsterCommand(new MenuEntry(F("tslInit"), CMD_EXACT, &BH1750Sensor::onCmdInit, F("")));
  initSensor();
  closeSensor();
}

void BH1750Sensor::onCmdInit(const char *ignore) {
  //BH1750Sensor.initSensor();
}

void BH1750Sensor::getData(LinkedList<Pair *> *data) {
  if (!initSensor()) return;
  data->add(new Pair("LUX", String(tsl->getLuxAutoScale())));
  closeSensor();
}

bool BH1750Sensor::initSensor() {
  bool init = false;
  tsl = new BH1750FVI();
  for (int i=0; i < 5; i++) {
    init = tsl->begin();
    if (init) break;
    i2cHigh();
    delay(100);
  }
  if (!init) {
    if (DEBUG) Serial << F("BH1750 - init failed!") << endl;
    return false;
  }
  SERIAL_PORT << F("Found BH1750 - LUX Sensor") << endl;
  return true;
}

void BH1750Sensor::closeSensor() {
  if (!tsl) return;
  delete tsl;
  tsl = NULL;
}
