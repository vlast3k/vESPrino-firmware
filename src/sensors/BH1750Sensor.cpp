#include <Wire.h>
#include "sensors/BH1750Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "common.hpp"
#include "BH1750FVI.h"

extern BH1750Sensor bh1750Sensor;
BH1750Sensor::BH1750Sensor() {
  registerSensor(this);
}

bool BH1750Sensor::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("bhTest"), CMD_EXACT, &BH1750Sensor::onCmdTest, F("")));
  if (initSensor()) return true;
  closeSensor();
  return false;
}

void BH1750Sensor::onCmdTest(const char *ignore) {
  bh1750Sensor.onCmdTestInst();
}

void BH1750Sensor::onCmdTestInst() {
  if (!initSensor()) return;
  tsl->SetSensitivity(1.00F);
  for (int i=0; i < 100; i++) {
    //LOGGER << F"AutoLux: "<< tsl->getLuxAutoScale() << endl;
    measureMT(31);
    measureMT(69);
    measureMT(254);
    LOGGER << endl;
    menuHandler.loop();
    delay(1000);
  }
  closeSensor();
}

void BH1750Sensor::getData(LinkedList<Pair *> *data) {
  if (!initSensor()) return;
  data->add(new Pair("LUX", String(tsl->getLuxAutoScale())));
  closeSensor();
}

bool BH1750Sensor::initSensor() {
  if (!rtcMemStore.hasSensor(RTC_SENSOR_BH1750)) return false;
  if (I2CHelper::i2cSDA ==  -1) return false;
  closeSensor();
  bool init = false;
  tsl = new BH1750FVI();
  for (int i=0; i < 5; i++) {
    init = tsl->begin();
    if (init) break;
    I2CHelper::i2cHigh();
    delay(100);
  }
  if (!init) {
    //if (DEBUG) LOGGER << F("BH1750 - init failed!") << endl;
    rtcMemStore.setSensorState(RTC_SENSOR_BH1750, false);
    return false;
  }
  //LOGGER << F("Found BH1750 - LUX Sensor") << endl;
  return true;
}

void BH1750Sensor::closeSensor() {
  if (!tsl) return;
  delete tsl;
  tsl = NULL;
}

void BH1750Sensor::measure() {
  tsl->SetMode(Continuous_H);
  delay(600);
  LOGGER << "\tH: " << tsl->GetLux();
  LOGGER.flush();

  tsl->SetMode(Continuous_H2);
  delay(600);
  LOGGER << ",\tH2: " << tsl->GetLux();
  LOGGER.flush();

  tsl->SetMode(Continuous_L);
  delay(60);
  LOGGER << ",\tL: " << tsl->GetLux();
  LOGGER.flush();

}

void BH1750Sensor::measureMT(uint8_t mt) {
  tsl->SetMTReg(mt);
  LOGGER.printf("MT=%3d : ", mt);
  measure();
  LOGGER << endl;
}
