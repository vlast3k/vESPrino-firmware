#include <Wire.h>
#include "sensors\CDM7160Sensor.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"
#include "common.hpp"

CDM7160Sensor::CDM7160Sensor() {
  registerSensor(this);
}

void CDM7160Sensor::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("cdmtest"), CMD_BEGIN, &CDM7160Sensor::onCmdTest, F("cdmtest b - b for debug - test CDM7160 sensor")));
}

void CDM7160Sensor::onCmdTest(const char *ignore) {
  Serial << F("Read CO2: ") <<  readCO2(strchr(ignore, 'b') != NULL) << endl;;
}

void CDM7160Sensor::getData(LinkedList<Pair *> *data) {
  int ppm = readCO2(true);
  if (ppm < 0) {
    menuHandler.scheduleCommand("scani2c");
    return;
  }
  data->add(new Pair("CO2", String(ppm)));
}

int CDM7160Sensor::readCO2(bool debug) {
  for (int h = 0; h < 8; h++) {
    byte addr = 0x69;
    //Wire.begin(D7, D5);
    //Wire.begin(D5, D7);
    Wire.beginTransmission(addr);
    Wire.write(0x1);
    int r = Wire.endTransmission(false);

    if (debug) { Serial << F("End trans: ") << r << endl; }

    if (r != 0) { delay(10);continue;}
    delay(10); // without this there is no data read
    //int r = Wire.requestFrom(addr, (byte)5);
    Wire.requestFrom((uint8_t)addr, (size_t)4, (bool)false);
    byte data[22];
    for (int i=0; i < 50 && !Wire.available(); i++) {
      delay(100);
      if (debug) Serial << ".";
    }
    //delay(100);
    if (debug) Serial << F("available = ") << Wire.available()<< endl;

    for (int i=0; i < 4; i++) {
      //Wire.requestFrom((uint8_t)addr, (size_t)1, (bool)false);
      data[i] = Wire.read();
      if (debug) Serial << _HEX(data[i]) << F(",");
      delay(5);
    }
//    Wire.endTransmission(true);
    if (data[1] == 0xFF || (data[1]&0x80) > 0) {
      if (debug) Serial << F("Sensor busy") << endl;
      delay(500);
      continue;
    }
    int ppm = (int)data[3]*0xFF + data[2];
    if (debug) {
      Serial << F("is busy   : ") << (data[1]&0x80) << endl;
      Serial << F("av wokring: ") << (data[1]&0x10) << endl;
      Serial << ppm << F(" ppm") << endl;
    }
    return ppm;
  }
  return -1;
}
