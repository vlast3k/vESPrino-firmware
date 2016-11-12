
#include "lib/Adafruit_BMP085.h"
#include <Wire.h>
#include "sensors/BMP085Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "common.hpp"

BMP085Sensor::BMP085Sensor() {
  registerSensor(this);
}

void BMP085Sensor::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("bmpInit"), CMD_EXACT, &BMP085Sensor::onCmdInit, F("")));
  bmp085Sensor.initSensor();

}

void BMP085Sensor::onCmdInit(const char *ignore) {
  bmp085Sensor.initSensor();
}

void BMP085Sensor::getData(LinkedList<Pair *> *data) {
  //Serial << "BME280 get Data" << endl;
   delay(10);
  //if (millis() < 10000) return; //give time for the BM

   if (!initSensor()) return;

   data->add(new Pair("TEMP", String(bme->readTemperature())));
   data->add(new Pair("PRES", String(bme->readPressure() / 100.0F)));
   //data->add(new Pair("ALT", String(bme->readAltitude(SEALEVELPRESSURE_HPA))));

   closeSensor();
    // Serial << "end BME280" << endl;
}

bool BMP085Sensor::initSensor() {
  if (i2cSDA == -1) return false;
  closeSensor();
  bme = new Adafruit_BMP085();
  //BME280->reset();
  bool init = false;
  for (int i=0; i < 5; i++) {
    init = bme->begin();
    if (init) break;
    i2cHigh();
    delay(100);

  }
  if (!init) {
    if (DEBUG) Serial << F("BMP085 - init failed!") << endl;
    delete bme;
    bme = NULL;
    return false;
  }
  SERIAL_PORT << F("Found BMP085 - Temperature/Pressure Sensor") << endl;
  return true;
}

void BMP085Sensor::closeSensor() {
  if (!bme) return;
  delete bme;
  bme = NULL;
}