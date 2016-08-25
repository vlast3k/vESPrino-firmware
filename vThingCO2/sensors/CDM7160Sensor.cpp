#include <Wire.h>
#include "sensors\CDM7160Sensor.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"
#include "common.hpp"
#include  <brzo_i2c.h>

CDM7160Sensor::CDM7160Sensor() {
  registerSensor(this);
}

void CDM7160Sensor::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("cdmtest"), CMD_BEGIN, &CDM7160Sensor::onCmdTest, F("cdmtest b - b for debug - test CDM7160 sensor")));
  handler->registerCommand(new MenuEntry(F("cdmreg"), CMD_BEGIN, &CDM7160Sensor::onChangeReg, F("cdmreg \"regid\" \"value\"")));
  configureSensor();
}

void CDM7160Sensor::getData(LinkedList<Pair *> *data) {
  int ppm = readCO2AutoRecover();
  if (ppm > 0) data->add(new Pair("CO2", String(ppm)));
}

int CDM7160Sensor::readCO2AutoRecover() {
  int ppm=0;
  for (int i=0; i < 5; i++) {
    int a = readCO2Raw(true);
    Serial <<"Raw CO2: " << a << endl;
    if (a > 0) ppm+=a;
    else {
      if (a == -1) menuHandler.scheduleCommand("scani2c");
      return -1;
    }
    delay(2000);
  }
  ppm = ppm / 5;
  // int ppm = readCO2Raw(true);
  // if (ppm < 0) {
  //   if (ppm == -1) menuHandler.scheduleCommand("scani2c");
  // }
  return ppm;
}

void CDM7160Sensor::configureSensor() {
  if (Wire.status() != I2C_OK) {
    Serial << F("I2C Bus status: ") << Wire.status() << endl;
    return;
  }
  uint8_t ctl = readI2CByte(CDM_CTL_REG);
  uint8_t avg = readI2CByte(CDM_AVG_REG);
  if ((ctl & CDM_FMODE) > 0)  writeByte(CDM_CTL_REG, 0x02);
  if (avg != CDM_AVG_DEFAULT) writeByte(CDM_AVG_REG, CDM_AVG_DEFAULT);
  if ((ctl & CDM_FMODE) > 0 || avg != CDM_AVG_DEFAULT) {
    Serial << F("Sensor Configuration updated") << endl;
  }
}

void CDM7160Sensor::onCmdTest(const char *ignore) {
  uint8_t rst = readI2CByte(0);
  Serial << "rst is: " << _HEX(rst) << endl;
  // if (rst == (uint8_t)0xFF) {
  //   Serial << " FAILED TO COMMUNICATE WITH SENSOR. Restarting I2C" << endl;
  //   menuHandler.scheduleCommand("scani2c");
  //Serial << " Wire.status = " << Wire.status() <<endl;
  Serial << "Register 0x00 : " << _BIN(readI2CByte(0)) << endl;
  delay(100);
  //Serial << " Wire.status = " << Wire.status() <<endl;
  Serial << "Register 0x01 : " << _BIN(readI2CByte(1)) << endl;
  delay(100);
  //Serial << " Wire.status = " << Wire.status() <<endl;
  Serial << "Register 0x02 : " << _BIN(readI2CByte(2)) << endl;
  delay(100);
  //Serial << " Wire.status = " << Wire.status() <<endl;
  Serial << "Register 0x03 : " << _BIN(readI2CByte(3)) << endl;
  delay(100);
  //Serial << " Wire.status = " << Wire.status() <<endl;
  Serial << "Register 0x04 : " << _BIN(readI2CByte(4)) << endl;
  delay(100);
  //Serial << " Wire.status = " << Wire.status() <<endl;
  Serial << "Register 0x07 : " << _BIN(readI2CByte(7)) << endl;
  delay(100);
  //Serial << " Wire.status = " << Wire.status() <<endl;
  Serial << F("Read CO2: ")  <<  readCO2AutoRecover() << endl;;
}

void CDM7160Sensor::onChangeReg(const char *line) {
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

  onCmdTest("");
}

int CDM7160Sensor::readCO2Raw(bool debug) {
  uint8_t buf[5];
  if (Wire.status() != I2C_OK) {
    if (debug) Serial << F("I2C Bus status: ") << Wire.status() << endl;
    return -1;
  }
  for (int i=0; i < 5; i++) {
    if (!readI2CBytes(0, buf, sizeof(buf))) {
      if (debug) Serial << F("Could not connect to Sensor") << endl;
      return -1;
    }
    if (buf[0] == (uint8_t) 0xFF) {
      if (debug) Serial << F("I2C Bus failed") << endl;
      return -1;
    }
    if ((buf[2] & 0x80) > 0) {
      if (debug) Serial << F("Sensor busy") << endl;
      delay(300);
      continue;
    }
    if ((buf[2] & 0x10) == 0) {
      if (debug) Serial << F("Averaging not yet completed") << endl;
    //  delay(500);
    //  continue;
      return -2;
    }
    int ppm = (int)buf[4]*0xFF + buf[3];
    return ppm;
  }
  return -2;
}

uint8_t CDM7160Sensor::readI2CByte(int reg) {
  uint8_t buf[1] = {0xFF};
  readI2CBytes(reg, buf, 1);
  return buf[0];
}

bool CDM7160Sensor::readI2CBytes(int start, uint8_t *buf, int len) {
  //return readI2CBytesBrzo(start, buf, len);
  for (int tr = 0; tr < 6; tr ++) {
    Wire.beginTransmission(CDM_ADDR_WRITE);
    Wire.write(start);
    int et = Wire.endTransmission(false);
    delay(10);
    if (et !=0) {
      Serial << et;
      delay(100);
      continue;
    }
    if (Wire.requestFrom(CDM_ADDR_WRITE, (size_t)len, (bool)false) < len) {
      Serial << "x";
      continue;
    }
    while (len -- > 0)   *(buf++) = Wire.read();
    return true;
  }
  return false;
}

bool CDM7160Sensor::readI2CBytesBrzo(int start, uint8_t *buf, int len) {
  for (int tr = 0; tr < 6; tr ++) {
    brzo_i2c_start_transaction(CDM_ADDR_WRITE, 400);
    byte xx[1];
    xx[0] = start;
    brzo_i2c_write(xx, 1, true);
    brzo_i2c_read(buf, len, false);
    int et = brzo_i2c_end_transaction();
    Serial << "brzo end: " << et <<endl;
    return true;
  }
  return false;
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

// void CDM7160Sensor::startChangeMode() {
//   writeCfg(0x01, 0x2, false);
// }
// void CDM7160Sensor::endChangeMode() {
//   writeCfg(0x01, 0x2, true);
// }
// void CDM7160Sensor::writeCfg(uint8_t reg, uint8_t bit, bool value) {
//   uint8_t currentValue = readI2CByte(reg);
//   delay(100);
//   Serial << "Change reg: " << reg << ", old value: " << _BIN(currentValue) << endl;
//   if (value) {
//     currentValue = currentValue | bit;
//   } else {
//     //bit ^= 0xFF;
//     currentValue = currentValue & (~bit);
//   }
//   Serial << "Change reg: " << reg << ", will set : " << _BIN(currentValue) << endl;
//
//   writeByte(reg, currentValue);
//   delay(100);
//   currentValue = readI2CByte(reg);
//   Serial << "Change reg: " << reg << ", currentue: " << _BIN(currentValue) << endl;
//   delay(100);
// }
//
//
//
//

//
//   for (int h = 0; h < 8; h++) {
//     byte addr = 0x69;
//     //Wire.begin(D7, D5);
//     //Wire.begin(D5, D7);
//     Wire.beginTransmission(addr);
//     Wire.write(0x1);
//     int r = Wire.endTransmission(false);
//
//     if (debug) { Serial << F("End trans: ") << r << endl; }
//
//     if (r != 0) { delay(10);continue;}
//     delay(10); // without this there is no data read
//     //int r = Wire.requestFrom(addr, (byte)5);
//     Wire.requestFrom((uint8_t)addr, (size_t)4, (bool)false);
//     byte data[22];
//     for (int i=0; i < 50 && !Wire.available(); i++) {
//       delay(100);
//       if (debug) Serial << ".";
//     }
//     //delay(100);
//     if (debug) Serial << F("available = ") << Wire.available()<< endl;
//
//     for (int i=0; i < 4; i++) {
//       //Wire.requestFrom((uint8_t)addr, (size_t)1, (bool)false);
//       data[i] = Wire.read();
//       if (debug) Serial << _HEX(data[i]) << F(",");
//       delay(5);
//     }
// //    Wire.endTransmission(true);
//     if (data[1] == 0xFF || (data[1]&0x80) > 0) {
//       if (debug) Serial << F("Sensor busy") << endl;
//       delay(500);
//       continue;
//     }
//     int ppm = (int)data[3]*0xFF + data[2];
//     if (debug) {
//       Serial << F("is busy   : ") << (data[1]&0x80) << endl;
//       Serial << F("av wokring: ") << (data[1]&0x10) << endl;
//       Serial << ppm << F(" ppm") << endl;
//     }
//     return ppm;
//   }
//   return -1;
// }
