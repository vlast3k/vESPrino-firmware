#include <Wire.h>
#include "sensors/CDM7160Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "common.hpp"
CDM7160Sensor::CDM7160Sensor() {
  registerSensor(this);
}

bool CDM7160Sensor::setup(MenuHandler *handler) {
  if (!rtcMemStore.hasSensor(RTC_SENSOR_CDM7160)) {
    hasSensor = false;
    return false;
  }
  handler->registerCommand(new MenuEntry(F("cdmloop"), CMD_BEGIN, &CDM7160Sensor::onCmdLoop, F("cdmtest b - b for DEBUG - test CDM7160 sensor")));
  handler->registerCommand(new MenuEntry(F("cdmtest"), CMD_BEGIN, &CDM7160Sensor::onCmdTest, F("cdmtest b - b for DEBUG - test CDM7160 sensor")));
  handler->registerCommand(new MenuEntry(F("cdmswitch"), CMD_BEGIN, &CDM7160Sensor::onCdmSwitch, F("cdmtest b - b for DEBUG - test CDM7160 sensor")));
  handler->registerCommand(new MenuEntry(F("cdmreg"), CMD_BEGIN, &CDM7160Sensor::onChangeReg, F("cdmreg \"regid\" \"value\"")));
  handler->registerCommand(new MenuEntry(F("cdmperf"), CMD_BEGIN, &CDM7160Sensor::onPerf, F("cdmreg \"regid\" \"value\"")));
  if (I2CHelper::i2cSDA > -1 && I2CHelper::checkI2CDevice(CDM_ADDR_WRITE)) {
    //LOGGER << F("Found CDM7160 CO2 Sensor") << endl;
    if (!PowerManager.isWokeFromDeepSleep()) configureSensor();
    hasSensor = true;
  } else {
    rtcMemStore.setSensorState(RTC_SENSOR_CDM7160, false);
  }
  return hasSensor;
}

void CDM7160Sensor::onPerf(const char *ignore) {
  for (int i=0; i<100; i++) {
    LOGGER << I2CHelper::checkI2CDevice(CDM_ADDR_WRITE);
    delay(5);
  }
  LOGGER << endl;

}

void CDM7160Sensor::getData(LinkedList<Pair *> *data) {
  if (!hasSensor) return;

  //if (!checkI2CDevice(CDM_ADDR_WRITE)) return;
//  for (int i=0; i < 3; i++) {
  int ppm = readCO2AutoRecover();
  if (ppm > 0) {
    int weight = 1;
    int diff = abs(ppm - rtcMemStore.getAverage());
    if (diff > 100) weight = 6;
    else if (diff > 50) weight = 2;
    rtcMemStore.addAverageValue(ppm, weight);
  }
  //  delay(5000);
//  }
  //if (rtcMemStore.getAverage() > 1) data->add(new Pair("CO2", String(rtcMemStore.getAverage())));
  if (rtcMemStore.getAverage() < 10000 && rtcMemStore.getAverage() > 100) {
    data->add(new Pair("CO2", String(round(rtcMemStore.getAverage()))));
  }
  data->add(new Pair("CO2R", String(ppm)));

}

int CDM7160Sensor::readCO2AutoRecover() {
  int ppm=0;
  for (int i=0; i < 2; i++) {
    int a = readCO2Raw();
    // for (int k=0; k < 4 ; k++) {
    //   int b = readCO2Raw();
    //   if (a == b) break;
    //   a = b;
    // }
    //int a = readCO2Raw();
    LOGGER << F("Raw CO2: ") << a << endl;
    if (a > 0) return a;
    else if (a == -1) { //maybe some temprary error, retry once more before reseting bus
      delay(2000);
    } else if (a == -2) { // cannot recover that fast, e.g. averaging not completed
      return -2;
    }
  }
  menuHandler.scheduleCommand("scani2c");
  return -1;
}

void CDM7160Sensor::configureSensor() {
  // if (i2cWireStatus() != I2C_OK) {
  //   int stat = Wire.status();
  //   LOGGER << F("I2C Bus status: ") <<  Wire.status() << endl;
  //   return;
  // }
  Wire.setClock(10000);
  uint8_t ctl = readI2CByte(CDM_CTL_REG);
  uint8_t avg = readI2CByte(CDM_AVG_REG);
  if (ctl == 0xFF || avg == 0xFF) {
    LOGGER << F("ERROR: Could not read sensor config") << endl;
    return;
  }
  //if ((ctl & CDM_FMODE) > 0)  writeByte(CDM_CTL_REG, 0x02);
  if (ctl != 6) {
    writeByte(CDM_CTL_REG, 0x00);
    delay(100);
    writeByte(CDM_CTL_REG, 0x06);
    delay(100);
  }

  if (avg != CDM_AVG_DEFAULT) writeByte(CDM_AVG_REG, CDM_AVG_DEFAULT);
  if (ctl != 6 || avg != CDM_AVG_DEFAULT) {
    LOGGER << F("Sensor Configuration updated") << endl;
  }
}

void CDM7160Sensor::onCmdLoop(const char *ignore) {
  Wire.setClock(10000);
  LOGGER << F("Avg count: ") << readI2CByte(7) << endl;
  int last = 0;
  for (int i=0; i < 50; i++) {
    int co2 = readCO2Raw();
    char x[50];
    sprintf(x, String(F("%2d: %d (%d)\n")).c_str(), i, co2, co2-last);
    LOGGER.printf(x);
    LOGGER.flush();
    String s = "oled ";
    s += x;
    menuHandler.scheduleCommand(s.c_str());
    last = co2;
    menuHandler.loop();

    delay(2000);
    //menuHandler.processUserInput();
    //menuHandler.processUserInput();
    //menuHandler.loop();
    //LOGGER << i << "\t: " << readCO2Raw(true) << endl;
  }
}

void CDM7160Sensor::onCdmSwitch(const char *ignore) {
  Wire.setClock(10000);
  for (int k=0; k<50; k++) {
    int last = 0;
    writeByte(CDM_CTL_REG, 0x06);
    for (int i=0; i < 30; i++) {
      int co2 = readCO2Raw();
      if (co2 == 1) {
        i--;
        continue;
      }
      char x[50];
      sprintf(x, String(F("%2d: %d (%d)\n")).c_str(), i, co2, co2-last);
      LOGGER.printf(x);
      LOGGER.flush();
      //    menuHandler.loop();
      last = co2;
      delay(2000);
    }
    writeByte(CDM_CTL_REG, 0x00);
    delay(120000);
  }
}


void CDM7160Sensor::onCmdTest(const char *ignore) {
  Wire.setClock(10000);
  uint8_t rst = readI2CByte(0);
  LOGGER << F("rst is: ") << _HEX(rst) << endl;
  // if (rst == (uint8_t)0xFF) {
  //   LOGGER << " FAILED TO COMMUNICATE WITH SENSOR. Restarting I2C" << endl;
  //   menuHandler.scheduleCommand("scani2c");
  //LOGGER << " Wire.status = " << Wire.status() <<endl;
  LOGGER << F("Register 0x00 : ") << _BIN(readI2CByte(0)) << endl;
//  delay(100);
  LOGGER << F("Register 0x01 : ") << _BIN(readI2CByte(1)) << endl;
//  delay(100);
  LOGGER << F("Register 0x02 : ") << _BIN(readI2CByte(2)) << endl;
//  delay(100);
  LOGGER << F("Register 0x03 : ") << _BIN(readI2CByte(3)) << endl;
//  delay(100);
  LOGGER << F("Register 0x04 : ") << _BIN(readI2CByte(4)) << endl;
//  delay(100);
  LOGGER << F("Register 0x07 : ") << _BIN(readI2CByte(7)) << endl;
//  delay(100);
  LOGGER << F("Read CO2: ")  <<  readCO2AutoRecover() << endl;;
}

void CDM7160Sensor::onChangeReg(const char *line) {
  char creg[10], cval[10];
  line = extractStringFromQuotes(line, creg);
  line = extractStringFromQuotes(line, cval);
  uint8_t reg = atoi(creg);
  uint8_t val = atoi(cval);
  for (int i=0; i < 5; i++) {
    uint8_t curVal =  readI2CByte(reg);
    if (curVal == 0xFF) continue;
    //LOGGER << F("Before : ") << _HEX(curVal) << F(" : ") << _BIN(curVal) << endl;
    int ret = writeByte(reg, val);
    if (ret > 0) continue;
    curVal =  readI2CByte(reg);
    if (curVal == 0xFF) continue;
    //LOGGER << F("After : ") << _HEX(curVal) << F(" : ") << _BIN(curVal) << endl;
    break;
  }

  onCmdTest("");
}

int CDM7160Sensor::readCO2Raw() {
  Wire.setClock(10000);
  uint8_t buf[5];
  // if (i2cWireStatus() != I2C_OK) {
  //   if (DEBUG) LOGGER << F("I2C Bus status: ") << Wire.status() << endl;
  //   return -1;
  // }
  for (int i=0; i < 5; i++) {
    if (!readI2CBytes(0, buf, sizeof(buf))) {
      if (DEBUG) LOGGER << F("Could not connect to Sensor") << endl;
      return -1;
    }
    if (buf[0] == (uint8_t) 0xFF) {
      if (DEBUG) LOGGER << F("I2C Bus failed") << endl;
      return -1;
    }
    if ((buf[2] & 0x80) > 0) {
      if (DEBUG) LOGGER << F("Sensor busy") << endl;
      delay(300);
      continue;
    }
    if (buf[4] == 0xFF) {
      delay(10);
      continue;
    }
    int ppm = (int)buf[4]*0xFF + buf[3];

    if ((buf[2] & 0x10) == 0) {
      if (DEBUG) LOGGER << F("Averaging not yet completed: ") << ppm  << endl;
      //delay(500);
      //continue;
      return -2;
    } else {
      return ppm;
    }
    //int ppm = (int)buf[4]*0xFF + buf[3];

  }
  return -2;
}

uint8_t CDM7160Sensor::readI2CByte(int reg) {
  uint8_t buf[1] = {0xFF};
  readI2CBytes(reg, buf, 1);
  return buf[0];
}

bool CDM7160Sensor::readI2CBytes(int start, uint8_t *buf, int len) {
  I2CHelper::i2cWireStatus();
  I2CHelper::i2cHigh();
  for (int tr = 0; tr < 6; tr ++) {
    Wire.beginTransmission(CDM_ADDR_WRITE);
    //delay(10);
    Wire.write(start);
    //delay(10);
    int et = Wire.endTransmission(false);
    delay(20);//for some reason, w/o this nothing is received
    if (et !=0) {
       if (DEBUG) LOGGER << et;
       I2CHelper::i2cWireStatus();
       I2CHelper::i2cHigh();
       continue;
    }
    if (Wire.requestFrom(CDM_ADDR_READ, (size_t)len, (bool)false) < len) {
      if (DEBUG) LOGGER << F("x");
      continue;
    }
    while (len -- > 0)   *(buf++) = Wire.read();
    return true;
  }
  return false;
}

// bool CDM7160Sensor::readI2CBytesBrzo(int start, uint8_t *buf, int len) {
//   for (int tr = 0; tr < 6; tr ++) {
//     brzo_i2c_start_transaction(CDM_ADDR_WRITE, 400);
//     byte xx[1];
//     xx[0] = start;
//     brzo_i2c_write(xx, 1, true);
//     brzo_i2c_read(buf, len, false);
//     int et = brzo_i2c_end_transaction();
//     LOGGER << "brzo end: " << et <<endl;
//     return true;
//   }
//   return false;
// }
//
int CDM7160Sensor::writeByte(uint8_t reg, uint8_t value) {
  LOGGER << F("WriteByte :reg: ") << reg << F(", ") << _BIN(value) <<endl;
  byte addr = 0x69;
  int r;
  for (int i=0; i < 20; i++) {
    // if (i2cWireStatus() != I2C_OK) {
    //   LOGGER << F("I2C Bus status: ") <<  Wire.status() << endl;
    // }
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(value);
    // if (i2cWireStatus() != I2C_OK) {
    //   LOGGER << F("I2C Bus status: ") <<  Wire.status() << endl;
    // }
    r = Wire.endTransmission(true);
    delay(10);
    if (r == 0) break;
    LOGGER << F("End trans : ") << r << endl;
    LOGGER.flush();
    I2CHelper::i2cWireStatus();
    I2CHelper::i2cHigh();
    delay(10);
    //Wire.begin();
  }
  LOGGER << F("End trans : ") << r << endl;
  return r;
}
