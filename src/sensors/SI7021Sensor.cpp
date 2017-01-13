#include <SI7021.h>
#include <Wire.h>
#include "sensors/SI7021Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "common.hpp"
#include <I2CHelper.hpp>

SI7021Sensor::SI7021Sensor() {
  registerSensor(this);
}

bool SI7021Sensor::setup(MenuHandler *handler) {
  //handler->registerCommand(new MenuEntry(F("siInit"), CMD_EXACT, &SI7021Sensor::onCmdInit, F("")));
  if (initSensor()) {
    String adjStr = PropertyList.readProperty(PROP_TEMP_ADJ);
    adj = atof(adjStr.c_str());
    closeSensor();
    return true;
  } else {
    closeSensor();
    return false;
  }

}

// void SI7021Sensor::onCmdInit(const char *ignore) {
//   si7021Sensor.initSensor();
// }

void SI7021Sensor::getData(LinkedList<Pair *> *data) {
  //LOGGER << "SI721 get Data" << endl;
  //delay(100);
   if (!initSensor()) return;
   double temp = si7021->readTemp();
   double adjTemp = temp + adj;
   String t1 = String(adjTemp);
   String t1r = String(temp);
   replaceDecimalSeparator(t1);
   replaceDecimalSeparator(t1r);
   data->add(new Pair("TEMP", t1));
   data->add(new Pair("TEMPR", t1r));
   data->add(new Pair("HUM", String((int)si7021->readHumidity())));
   closeSensor();
}

bool SI7021Sensor::initSensor() {
  //if (!rtcMemStore.hasSensor(RTC_SENSOR_SI7021)) return false;
  if (I2CHelper::i2cSDA ==  -1) return false;
  closeSensor();
  si7021 = new SI7021();
  yield();
  //si7021->reset();
  //si7021->begin(D1, D6); // Runs : Wire.begin() + reset()
  //LOGGER << "SI7021 init :" <<  si7021->getDeviceID() << endl;
  bool init = false;
  for (int i=0; i < 5; i++) {
      init = si7021->begin();
      if (init) break;
      delay(10);
  }

  if (!init) {
    delete si7021;
    si7021 = NULL;
    //rtcMemStore.setSensorState(RTC_SENSOR_SI7021, false);
    return false;
  }
  //LOGGER << F("Found SI7021 - Temperature/Humidity Sensor") << endl;
  //LOGGER.flush();
  if (si7021->getHumidityRes() != 12) si7021->setHumidityRes(12); // Humidity = 12-bit / Temperature = 14-bit
  return true;
}

void SI7021Sensor::closeSensor() {
  if (!si7021) return;
  delete si7021;
  si7021 = NULL;
}
