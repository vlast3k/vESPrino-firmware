#include <Adafruit_Sensor.h>
#include "Adafruit_BME280.h"
#include <Wire.h>
#include "sensors/BME280Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "common.hpp"

BME280Sensor::BME280Sensor() {
  registerSensor(this);
}

bool BME280Sensor::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("bmeInit"), CMD_EXACT, &BME280Sensor::onCmdInit, F("")));
  if (bme280Sensor.initSensor()) return true;
  closeSensor();
  return false;

}

void BME280Sensor::onCmdInit(const char *ignore) {
  bme280Sensor.initSensor();
}

void BME280Sensor::getData(LinkedList<Pair *> *data) {
  //LOGGER << "BME280 get Data" << endl;
  // delay(10);
  //if (millis() < 10000) return; //give time for the BM

   if (!initSensor()) return;
   double temp = bme->readTemperature();
   String adj = PropertyList.readProperty(PROP_TEMP_ADJ);
   double adjTemp = temp + atof(adj.c_str());
   adjTemp -= 0.5F; //BME280 tends to be 0.5C higher
   data->add(new Pair("TEMP", String(adjTemp)));
   data->add(new Pair("TEMPR", String(temp)));
   data->add(new Pair("HUM", String(bme->readHumidity())));
   data->add(new Pair("PRES", String(bme->readPressure() / 100.0F)));
   data->add(new Pair("ALT", String(bme->readAltitude(SEALEVELPRESSURE_HPA))));

   closeSensor();
    // LOGGER << "end BME280" << endl;
}

bool BME280Sensor::initSensor() {
  if (!rtcMemStore.hasSensor(RTC_SENSOR_BME280)) return false;
  if (I2CHelper::i2cSDA ==  -1) return false;
  closeSensor();
  bme = new Adafruit_BME280();
  //BME280->reset();
  bool init = false;
  for (int i=0; i < 5; i++) {
    init = bme->begin();
    if (init) break;
    I2CHelper::i2cHigh();
    delay(100);

  }
  if (!init) {
    //if (DEBUG) LOGGER << F("BME280 - init failed!") << endl;
    delete bme;
    bme = NULL;
    rtcMemStore.setSensorState(RTC_SENSOR_BME280, false);
    return false;
  }
  //LOGGER << F("Found BME280 - Temperature/Humidity/Pressure Sensor") << endl;
  LOGGER.flush();
  return true;
}

void BME280Sensor::closeSensor() {
  if (!bme) return;
  delete bme;
  bme = NULL;
}
