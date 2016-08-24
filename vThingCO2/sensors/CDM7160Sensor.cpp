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
  handler->registerCommand(new MenuEntry(F("cdmode"), CMD_BEGIN, &CDM7160Sensor::onChangeMode, F("cdmode")));
}

void CDM7160Sensor::onCmdTest(const char *ignore) {
  uint8_t rst = readI2CByte(0);
  Serial << "rst is: " << _HEX(rst) << endl;
  if (rst == (uint8_t)0xFF) {
    Serial << " FAILED TO COMMUNICATE WITH SENSOR. Restarting I2C" << endl;
    Serial << " Wire.status = " << Wire.status() << endl;
    menuHandler.scheduleCommand("scani2c");
    return;
  }
  Serial << "Register 0x00 : " << _BIN(readI2CByte(0)) << endl;
  Serial << "Register 0x01 : " << _BIN(readI2CByte(1)) << endl;
  Serial << "Register 0x02 : " << _BIN(readI2CByte(2)) << endl;
  Serial << "Register 0x03 : " << _BIN(readI2CByte(3)) << endl;
  Serial << "Register 0x04 : " << _BIN(readI2CByte(4)) << endl;
  Serial << "Register 0x07 : " << _BIN(readI2CByte(7)) << endl;
  Serial << F("Read CO2: ") <<  readCO2(strchr(ignore, 'b') != NULL) << endl;;
}

uint8_t CDM7160Sensor::readI2CByte(int reg) {
  uint8_t addr = 0x69;
  Wire.beginTransmission(addr);
  Wire.write(reg);
  int r = Wire.endTransmission(false);
  delay(10);
  Wire.requestFrom((uint8_t)addr, (size_t)1, (bool)false);
  uint8_t b = Wire.read();
  return b;
}

void CDM7160Sensor::startChangeMode() {
  writeCfg(0x01, 0x2, false);
}
void CDM7160Sensor::endChangeMode() {
  writeCfg(0x01, 0x2, true);
}

void CDM7160Sensor::writeByte(uint8_t reg, uint8_t value) {
  Serial << "WriteByte :reg: "<< reg << ", " << _BIN(value) <<endl;
  byte addr = 0x69;
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(value);
  delay(100);
  int r = Wire.endTransmission(true);
  delay(100);
  Serial << "End trans : " << r << endl;
}

void CDM7160Sensor::writeCfg(uint8_t reg, uint8_t bit, bool value) {
  uint8_t currentValue = readI2CByte(reg);
  delay(100);
  Serial << "Change reg: " << reg << ", old value: " << _BIN(currentValue) << endl;
  if (value) {
    currentValue = currentValue | bit;
  } else {
    //bit ^= 0xFF;
    currentValue = currentValue & (~bit);
  }
  Serial << "Change reg: " << reg << ", will set : " << _BIN(currentValue) << endl;

  writeByte(reg, currentValue);
  delay(100);
  currentValue = readI2CByte(reg);
  Serial << "Change reg: " << reg << ", currentue: " << _BIN(currentValue) << endl;
  delay(100);
}


void CDM7160Sensor::onChangeMode(const char *line) {
  char creg[10], cval[10];
  line = extractStringFromQuotes(line, creg);
  line = extractStringFromQuotes(line, cval);
  uint8_t reg = atoi(creg);
  uint8_t val = atoi(cval);
  uint8_t curVal =  readI2CByte(reg);
  Serial << "Before : " << _HEX(curVal) << " : " << _BIN(curVal) << endl;
  writeByte(reg, val);
  curVal =  readI2CByte(reg);
  Serial << "After : " << _HEX(curVal) << " : " << _BIN(curVal) << endl;
  // startChangeMode();
  // writeCfg(0x01, 0x4, true);
  // writeByte(0x01, 0x6);
  //endChangeMode();

  onCmdTest("");
  // byte addr = 0x69;
  // //Wire.begin(D7, D5);
  // //Wire.begin(D5, D7);
  // Serial << " CTL Register: " << readI2CByte(1) << endl;
  // Wire.beginTransmission(addr);
  // Wire.write(0x1);
  // Wire.write(0);
  // int r = Wire.endTransmission(true);
  // delay(100);
  //
  // Serial << " CTL Register, after change mode: " << readI2CByte(1) << endl;
  //
  // Wire.beginTransmission(addr);
  // Wire.write(0x1);
  // Wire.write(0);
  //  r = Wire.endTransmission(true);
  // delay(100);
  // Serial << " CTL Register, after set fmopde =0: " << readI2CByte(1) << endl;
  //
  // Wire.beginTransmission(addr);
  // Wire.write(0x1);
  // Wire.write(2);
  //  r = Wire.endTransmission(true);
  // delay(100);
  //
  // Serial << " CTL Register, after continous measurement: " << readI2CByte(1) << endl;
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
