#include <Wire.h>
#include "sensors\PM2005Sensor.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"
#include "common.hpp"

PM2005Sensor::PM2005Sensor() {
  registerSensor(this);
}

void PM2005Sensor::setup(MenuHandler *handler) {
//  handler->registerCommand(new MenuEntry(F("pm2005Init"), CMD_EXACT, &PM2005Sensor::onCmdInit, F("")));
  if (checkI2CDevice(0x28)) {
    Serial << F("Found PM2005 - Dust / Particle Sensor");
    hasSensor = true;
  }
}

// void PM2005Sensor::onCmdInit(const char *ignore) {
//   pm2005Sensor.initSensor();
// }

void PM2005Sensor::getData(LinkedList<Pair *> *data) {
  if (!hasSensor) return;
  //Serial << "PM2005 get Data" << endl;
  //delay(100);
  int pm25, pm10;
  if (!intReadData(pm25, pm10, false)) return;
  data->add(new Pair("PM25", String(pm25)));
  data->add(new Pair("PM10", String(pm10)));
  //Serial << "end PM2005" << endl;
}

bool PM2005Sensor::intBegin(int sda, int sca) {
  if (sda != 0 || sca != 0) Wire.begin(sda, sca);
  Wire.beginTransmission(0x28);
  Wire.write(0x51);
  return Wire.endTransmission(false) == 0;
}

bool PM2005Sensor::intReadData(int &pm25, int &pm10, bool debug) {
  if (!intBegin()) {
    if (debug) Serial << F("Failed to connect to PM2005\n");
    return false;
  }
  int r;
  byte data[22];
  byte cs = 0;
  r = Wire.requestFrom(0x28, 22, false);
  if (r != 22) {
    if (debug) Serial << F("Expected 22 bytes, but got ") << r << endl;
    return false;
  }
  for (int i=0; i < 22; i++) {
    data[i] = Wire.read();
    if (debug) Serial << _HEX(data[i]) << F(",");
    if (i < 21) cs ^= data[i];
  }
  if (debug) Serial << endl;
  if (cs != data[21]) {
    if (debug) Serial << F("Wrong Checksum: ") << cs << F(", expected: ") << data[21] << endl;
    return false;
  }
  if (debug) {
    Serial << F("Sensor Status: ") << data[2] << endl;
    Serial << F("PM 2.5 : ") << (data[5] << 2) + data[6] << endl;
    Serial << F("PM  10 : ") << (data[7] << 2) + data[8] << endl;
    Serial << F("Measuring Mode : ") << (data[9] << 2) + data[10] << endl;
  }
  pm25 = (data[5] << 2) + data[6];
  pm10 = (data[7] << 2) + data[8];
  return true;
}
