#include <Adafruit_Sensor.h>
#include "lib\Adafruit_BME280.h"
#include <Wire.h>
#include "sensors\BME280Sensor.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"
#include "common.hpp"

BME280Sensor::BME280Sensor() {
  registerSensor(this);
}

void BME280Sensor::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("bmeInit"), CMD_EXACT, &BME280Sensor::onCmdInit, F("")));
  bme280Sensor.initSensor();

}

void BME280Sensor::onCmdInit(const char *ignore) {
  bme280Sensor.initSensor();
}

void BME280Sensor::getData(LinkedList<Pair *> *data) {
  //Serial << "BME280 get Data" << endl;
   delay(10);
  //if (millis() < 10000) return; //give time for the BM

   if (!initSensor()) return;

   data->add(new Pair("TEMP", String(bme->readTemperature())));
   data->add(new Pair("HUM", String(bme->readHumidity())));
   data->add(new Pair("PRES", String(bme->readPressure() / 100.0F)));
   data->add(new Pair("ALT", String(bme->readAltitude(SEALEVELPRESSURE_HPA))));

   closeSensor();
    // Serial << "end BME280" << endl;
}

bool BME280Sensor::initSensor() {
  if (i2cSDA == -1) return false;
  closeSensor();
  bme = new Adafruit_BME280();
  //BME280->reset();
  bool init = false;
  for (int i=0; i < 5; i++) {
    init = bme->begin();
    if (init) break;
    i2cHigh();
    delay(100);

  }
  if (!init) {
    if (DEBUG) Serial << F("BME280 - init failed!") << endl;
    delete bme;
    bme = NULL;
    return false;
  }
  SERIAL_PORT << F("Found BME280 - Temperature/Humidity/Pressure Sensor") << endl;
  return true;
}

void BME280Sensor::closeSensor() {
  if (!bme) return;
  delete bme;
  bme = NULL;
}
