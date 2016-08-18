#include <Wire.h>
#include "sensors\CDM7160Sensor.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"
#include "common.hpp"

CDM7160Sensor::CDM7160Sensor() {
  registerSensor(this);
}

void CDM7160Sensor::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("cdmtest"), CMD_EXACT, &CDM7160Sensor::onCmdTest, F("")));

}

void CDM7160Sensor::onCmdTest(const char *ignore) {
  for (int h = 0; h < 8; h++) {
    byte addr = 0x69;
    //Wire.begin(D7, D5);
    //Wire.begin(D5, D7);
    Wire.beginTransmission(addr);
    Wire.write(0x3);
    int r = Wire.endTransmission(false);

    Serial << "End trans: " << r << endl;
    if (r != 0) { delay(10);continue;}
    //int r = Wire.requestFrom(addr, (byte)5);
    Wire.requestFrom((uint8_t)addr, (size_t)2, (bool)false);
    byte data[22];
    for (int i=0; i < 50 && !Wire.available(); i++) {
      delay(100);
      Serial << ".";
    }
    //delay(100);
    Serial << "available = " << Wire.available()<< endl;

    for (int i=0; i < 2; i++) {
      //Wire.requestFrom((uint8_t)addr, (size_t)1, (bool)false);
      data[i] = Wire.read();
      Serial << _HEX(data[i]) << F(",");
      delay(5);
    }
//    Wire.endTransmission(true);
    if (data[1] == 0xFF) {
      Serial << "Sensor busy" << endl;
      delay(500);
      continue;
    }
    Serial << (int)data[1]*0xFF + data[0] << " ppm" << endl;
    return;
  }
}

void CDM7160Sensor::getData(LinkedList<Pair *> *data) {
  Serial << "CDM7160 get Data" << endl;
  delay(100);
  int pm25, pm10;
  if (!intReadData(pm25, pm10, true)) return;
  data->add(new Pair("PM25", String(pm25)));
  data->add(new Pair("PM10", String(pm10)));
  Serial << "end CDM7160" << endl;
}

bool CDM7160Sensor::intBegin(int sda, int sca) {
  if (sda != 0 || sca != 0) Wire.begin(sda, sca);
  Wire.beginTransmission(0x28);
  Wire.write(0x51);
  return Wire.endTransmission(false) == 0;
}

bool CDM7160Sensor::intReadData(int &pm25, int &pm10, bool debug) {
  if (!intBegin()) {
    if (debug) Serial << F("Failed to connect to CDM7160\n");
    return false;
  }
  int r;
  byte data[22];
  byte cs = 0;
  r = Wire.requestFrom(0x28, 22, false);
  if (r != 22) {
    if (debug) Serial << "Expected 22 bytes, but got " << r << endl;
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
