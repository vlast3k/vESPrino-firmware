#include <SI7021.h>
#include <Wire.h>
#include "sensors/SI7021Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "common.hpp"

SI7021Sensor::SI7021Sensor() {
  registerSensor(this);
}

void SI7021Sensor::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("siInit"), CMD_EXACT, &SI7021Sensor::onCmdInit, F("")));
  initSensor();

}

void SI7021Sensor::onCmdInit(const char *ignore) {
  si7021Sensor.initSensor();
}

void SI7021Sensor::getData(LinkedList<Pair *> *data) {
  //Serial << "SI721 get Data" << endl;
  delay(100);
   if (!initSensor()) return;
   data->add(new Pair("TEMP", String(si7021->readTemp())));
   data->add(new Pair("HUM", String(si7021->readHumidity())));
   closeSensor();
    // Serial << "end si7021" << endl;
}

bool SI7021Sensor::initSensor() {
  if (i2cSDA == -1) return false;
  closeSensor();
  si7021 = new SI7021();
  //si7021->reset();
  //si7021->begin(D1, D6); // Runs : Wire.begin() + reset()
  //Serial << "SI7021 init :" <<  si7021->getDeviceID() << endl;
  bool init = false;
  for (int i=0; i < 5; i++) {
      init = si7021->begin();
      if (init) break;
      delay(10);
  }

  if (!init) {
    delete si7021;
    si7021 = NULL;
    return false;
  }
  SERIAL_PORT << F("Found SI7021 - Temperature/Humidity Sensor") << endl;
  si7021->setHumidityRes(8); // Humidity = 12-bit / Temperature = 14-bit
  return true;
}

void SI7021Sensor::closeSensor() {
  if (!si7021) return;
  delete si7021;
  si7021 = NULL;
}
