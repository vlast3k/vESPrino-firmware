#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>
#include "sensors\BME280Sensor.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"
#include "common.hpp"

BME280Sensor::BME280Sensor() {
  //registerSensor(this);
}

void BME280Sensor::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("bmeInit"), CMD_EXACT, &BME280Sensor::onCmdInit, F("")));

}

void BME280Sensor::onCmdInit(const char *ignore) {
  bme280Sensor.initSensor();
}

void BME280Sensor::getData(LinkedList<Pair *> *data) {
  Serial << "BME280 get Data" << endl;
  delay(100);
   if (!initSensor()) return;
   data->add(new Pair("TEMP", String(bme->readTemperature())));
   data->add(new Pair("HUM", String(bme->readHumidity())));
   data->add(new Pair("PRES", String(bme->readPressure() / 100.0F)));
   data->add(new Pair("ALT", String(bme->readAltitude(SEALEVELPRESSURE_HPA))));

   closeSensor();
     Serial << "end BME280" << endl;
}

bool BME280Sensor::initSensor() {
  bme = new Adafruit_BME280();
  //BME280->reset();
  bool init = bme->begin(); // Runs : Wire.begin() + reset()
  if (!init) {
    Serial <<"failed" << endl;
    delete bme;
    bme = NULL;
    return false;
  }
  SERIAL << "Found BME280 Temperature/Humidity Sensor-----\n" << endl;
  return true;
}

void BME280Sensor::closeSensor() {
  if (!bme) return;
  delete bme;
  bme = NULL;
}
