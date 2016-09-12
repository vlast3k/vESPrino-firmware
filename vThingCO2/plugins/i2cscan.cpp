#include <Arduino.h>
#include <Streaming.h>
#include <Wire.h>
#include "common.hpp"
#include  <brzo_i2c.h>

bool checkI2CDevice(int addr) {
  for (int i=0; i < 4; i++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) return true;
    delay(10);
  }
  return false;
}

bool checkI2CDevice(int sda, int sca, int addr) {
  Wire.begin(sda, sca);
  Wire.beginTransmission(addr);
  int res = Wire.endTransmission();
  SERIAL_PORT << F("i2c addr:") << addr << F(", i2c res: ") << _HEX(res) << endl;
  return !res;
}

void i2cPrintAddr(byte address) {
  if (address<16) Serial.print(F("0"));
  Serial << _HEX(address) << endl;
}
void dumpI2CBus(const char *line) {
  Wire.begin(D5, D7);
  byte error, address;
  int nDevices;
  for(address = 1; address < 0xff; address++ )  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    Serial << "Address 0x" << _HEX(address) << " = " << error << endl;
    if ((address%10)==0) delay(1);
  }
}

bool hasI2CDevices(int sda, int sca, String &sda_str, String &sca_str, bool debug) {
  Wire.begin(sda, sca);
  byte error, address;
  int nDevices;
//  if (debug) Serial.printf(String(F("Scanning SDA:SCA = %s:%s\n")).c_str(), sda_str.c_str(), sca_str.c_str());//.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 0xff; address++ )  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0) {
      if (debug) {
        Serial.print(F("I2C device found at address 0x"));
        i2cPrintAddr(address);
      }
      nDevices++;
    } else if (error==4) {
      delay(1);
      if (debug) {
        Serial.print(F("Unknow error at address 0x"));
        i2cPrintAddr(address);
      }
    }
  }
  if (debug) {
    if (nDevices == 0)  Serial.println(F("No I2C devices found\n"));
    else                Serial.println(F("done\n"));
  }
  return nDevices > 0;
}

bool findI2C(int &sda, int &scl, bool debug) {
  int gpios[] = {D1, D5};
  String gpios_str[] = {F("D1"), F("D5")};
  int size = sizeof(gpios) / sizeof(int);
  bool found = false;
  for (int i=0; i < size; i++) {
    for (int k=0; k < size; k++) {
      if (i == k) continue;
    //  if (debug) Serial << "Scanning " << gpios_str[sda], << " : " << sca_str << endl;
      if (hasI2CDevices(gpios[i], gpios[k], gpios_str[i], gpios_str[k], debug)) {
        sda = gpios[i];
        scl = gpios[k];
        Serial.printf(String(F("Found i2c bus on SDA:SCL = %s:%s (%d:%d)\n")).c_str(), gpios_str[i].c_str(), gpios_str[k].c_str(), sda, scl);
        return true;
      }
      delay(1);
    }
  }
  return false;
}

void cmdScanI2C(const char *ignore) {
  //if ()
   //pinMode(D8, OUTPUT);    //enable power via D8
  //  digitalWrite(D8, LOW);
  //  delay(500);
//   digitalWrite(D8, HIGH);
//   delay(1000);
  // pinMode(D8, INPUT);
  //brzo_i2c_setup(D5, D1, 50000);
  int a, b;
  findI2C(a, b, true);
}

void beginI2C() {
//  brzo_i2c_setup(D5, D1, 50000);
  //Wire.begin(D5, D1);
   if (findI2C(i2cSDA, i2cSCL, false)) {
     Wire.begin(i2cSDA, i2cSCL);
  //   Wire.
   } else {
     Serial << F("No I2C Devices found\n");
   }
}
