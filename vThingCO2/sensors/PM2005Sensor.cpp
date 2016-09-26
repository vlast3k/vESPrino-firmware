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
    Serial << F("Found PM2005 - Dust / Particle Sensor\n");
    hasSensor = true;
    int pm25, pm10, mode, status;
    if (!intReadData(pm25, pm10, status, mode)) return;
    if (mode != 5) {
      setDynamicMode();
      intReadData(pm25, pm10, status, mode);
    }
  }
}

// void PM2005Sensor::onCmdInit(const char *ignore) {
//   pm2005Sensor.initSensor();
// }

void PM2005Sensor::getData(LinkedList<Pair *> *data) {
  if (!hasSensor) return;
  //Serial << "PM2005 get Data" << endl;
  //delay(100);
  int pm25, pm10, mode, status;
  if (!intReadData(pm25, pm10, status, mode)) return;
  data->add(new Pair("PM25", String(pm25)));
  data->add(new Pair("PM10", String(pm10)));
  //Serial << "end PM2005" << endl;
}

bool PM2005Sensor::intBegin() {
//  if (sda != 0 || sca != 0) Wire.begin(sda, sca);
  //Wire.status();
  for (int i=0; i < 5; i++) {
    Wire.beginTransmission(0x28);
    Wire.write(0x51);
    if (!Wire.endTransmission(false)) return true;
    delay(100);
  }
  return false;
}

void PM2005Sensor::setDynamicMode() {
  //Wire.status();
  int res = 3;
  for (int i=0; i<5; i++) {
    Wire.beginTransmission(0x28);
    Wire.write(0x50);

    Wire.write(0x16);
    Wire.write(7);
    Wire.write(5);
    Wire.write(0);
    Wire.write(0);
    Wire.write(0);
    Wire.write(0x14);
    res = Wire.endTransmission();
    if (res == 0) break;
    delay(100);
  }
  if (DEBUG) Serial << F("PM2005 setDynamicMode res = ") << res << endl;
}

bool PM2005Sensor::intReadData(int &pm25, int &pm10, int &status, int &mode) {
  if (!intBegin()) {
    if (DEBUG) Serial << F("\nFailed to connect to PM2005\n");
    return false;
  }
  int r;
  byte data[22];
  byte cs = 0;
  r = Wire.requestFrom((uint8_t)0x28, (size_t)22, false);
  if (r != 22) {
    if (DEBUG) Serial << F("Expected 22 bytes, but got ") << r << endl;
    return false;
  }
  for (int i=0; i < 22; i++) {
    data[i] = Wire.read();
    if (DEBUG) Serial << _HEX(data[i]) << F(",");
    if (i < 21) cs ^= data[i];
  }
  if (DEBUG) Serial << endl;
  if (cs != data[21]) {
    if (DEBUG) Serial << F("Wrong Checksum: ") << cs << F(", expected: ") << data[21] << endl;
    return false;
  }
  status = data[2];
  pm25 = (data[5] << 2) + data[6];
  pm10 = (data[7] << 2) + data[8];
  mode = (data[9] << 2) + data[10];
  if (DEBUG) {
    Serial << F("Sensor Status: ") << status << endl;
    Serial << F("PM 2.5 : ") << pm25 << endl;
    Serial << F("PM  10 : ") << pm10 << endl;
    Serial << F("Measuring Mode : ") << mode << endl;
  }
  return true;
}
