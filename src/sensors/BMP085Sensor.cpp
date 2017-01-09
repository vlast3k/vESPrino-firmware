
#include "Adafruit_BMP085.h"
#include <Wire.h>
#include "sensors/BMP085Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "common.hpp"
#include <I2CHelper.hpp>

BMP085Sensor::BMP085Sensor() {
  registerSensor(this);
}

bool BMP085Sensor::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("bmpInit"), CMD_EXACT, &BMP085Sensor::onCmdInit, F("")));
  if (bmp085Sensor.initSensor()) return true;
  closeSensor();
  return false;

}

void BMP085Sensor::onCmdInit(const char *ignore) {
  bmp085Sensor.initSensor();
}

void BMP085Sensor::getData(LinkedList<Pair *> *data) {
  //LOGGER << "BME280 get Data" << endl;
  // delay(10);
  //if (millis() < 10000) return; //give time for the BM

   if (!initSensor()) return;

   double temp = bme->readTemperature();
   String adj = PropertyList.readProperty(PROP_TEMP_ADJ);
   double adjTemp = temp + atof(adj.c_str());
   adjTemp += 0.5F; //BMP180 tends to be 0.5C lower
   data->add(new Pair("TEMP", String(adjTemp)));
   data->add(new Pair("TEMPR", String(temp)));

   data->add(new Pair("PRES", String(bme->readPressure() / 100.0F)));
   //data->add(new Pair("ALT", String(bme->readAltitude(SEALEVELPRESSURE_HPA))));

   closeSensor();
    // LOGGER << "end BME280" << endl;
}

bool BMP085Sensor::initSensor() {
  if (!rtcMemStore.hasSensor(RTC_SENSOR_BMP180)) return false;
  if (I2CHelper::i2cSDA ==  -1) return false;
  closeSensor();
  bme = new Adafruit_BMP085();
  //BME280->reset();
  bool init = false;
  for (int i=0; i < 5; i++) {
    init = bme->begin();
    if (init) break;
    I2CHelper::i2cHigh();
    delay(100);

  }
  if (!init) {
    //if (DEBUG) LOGGER << F("BMP085 - init failed!") << endl;
    delete bme;
    bme = NULL;
    rtcMemStore.setSensorState(RTC_SENSOR_BMP180, false);

    return false;
  }
  //LOGGER << F("Found BMP085 - Temperature/Pressure Sensor") << endl;
  LOGGER.flush();
  return true;
}

void BMP085Sensor::closeSensor() {
  if (!bme) return;
  delete bme;
  bme = NULL;
}
