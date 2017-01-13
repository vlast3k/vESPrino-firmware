#include <I2CHelper.hpp>
#include <Arduino.h>
#include <Stream.h>
#include <Streaming.h>
#include <Wire.h>
#include <RTCMemStore.hpp>



int8_t I2CHelper::i2cSDA = -1;
int8_t I2CHelper::i2cSCL = -1;
Stream *I2CHelper::LOGGER = NULL;
extern RTCMemStore rtcMemStore;

bool I2CHelper::checkI2CDevice(int addr) {
  i2cHigh();
  for (int i=0; i < 10; i++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) return true;
    delay(10);
  }
  return false;
}

bool I2CHelper::checkI2CDevice(int sda, int sca, int addr) {
  Wire.begin(sda, sca);
  Wire.beginTransmission(addr);
  int res = Wire.endTransmission();
  *LOGGER << F("i2c addr:") << addr << F(", i2c res: ") << _HEX(res) << endl;
  return !res;
}

void I2CHelper::i2cPrintAddr(byte address) {
  if (address<16) LOGGER->print(F("0"));
  *LOGGER << _HEX(address) << endl;
}
void I2CHelper::dumpI2CBus(const char *line) {
  //Wire.begin(D5, D7);
  byte error, address;
  int nDevices;
  for(address = 1; address < 0xff; address++ )  {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    *LOGGER << "Address 0x" << _HEX(address) << " = " << error << endl;
    if ((address%10)==0) delay(1);
  }
}

bool I2CHelper::hasI2CDevices(int sda, int sca, String &sda_str, String &sca_str, bool debug) {
  Wire.begin(sda, sca);
  //Wire.setClock(20000);
  byte error, address;
  int nDevices;
  if (debug) LOGGER->printf(String(F("Scanning SDA:SCA = %s:%s\n")).c_str(), sda_str.c_str(), sca_str.c_str());//.println("Scanning...");
  //
  // pinMode(sda, HIGH);
  // pinMode(sca, HIGH);
  // delay(100);
  nDevices = 0;
  bool busOk = true;
  int numErr = 0;
  uint32_t wdtPrev = millis();
  for(address = 1; busOk && address < 0xff; address++ )  {
    if (millis() - wdtPrev > 6) {
      delay(1);
      wdtPrev = millis();
    }

    //*LOGGER << ".";
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.

    for (int i=0; i < 5; i++) {
      Wire.beginTransmission(address);
      error = slowEndTransmission();

      if (error == 0) {
        if (debug) {
          LOGGER->print(F("I2C device found at address 0x"));
          i2cPrintAddr(address);
        }
        nDevices++;
        break;
      } else if (error==4) {
        //delay(1);
        if (numErr++ == 7) {
          nDevices = 0;
          busOk = false;
          break;
        }
        if (debug) {
          LOGGER->print(F("SDA Line High 0x"));
          i2cPrintAddr(address);
        }
      }
      // else {
      //   if (debug) {
      //     *LOGGER.print(F("I2C Error on 0x"));
      //     i2cPrintAddr(address);
      //   }
      // }
    }
  }
  if (debug) {
    if (nDevices == 0)  LOGGER->println(F("No I2C devices found\n"));
    else                LOGGER->println(F("done\n"));
  }
  return nDevices > 0;
}

int I2CHelper::i2cWireStatus() {
  for (int i=0; i < 5 && Wire.status() != I2C_OK; i++) delay(5);
  delay(10);
  return Wire.status();

}



bool I2CHelper::findI2C(int8_t &sda, int8_t &scl, long disabledPorts, bool debug) {
//  int gpios[] = {D1, D5, D2, D5, D6};
  int gpios[] = {D1, D5, D7, D6};
  String gpios_str[] = {F("D1"), F("D5"), F("D7"), F("D6")};
  int size = sizeof(gpios) / sizeof(int);
  bool found = false;
  for (int i=0; i < size; i++) {
    for (int k=0; k < size; k++) {
      if (i == k) continue;
      // *LOGGER << "PortDisabled: " << gpios[i] <<  " "<<isBitSet(disabledPorts, gpios[i]) << " " << disabledPorts << endl;
      // *LOGGER << "PortDisabled: " << gpios[k] <<  " "<<isBitSet(disabledPorts, gpios[k]) << " " << disabledPorts << endl;
      if (isBitSet(disabledPorts, gpios[i]) || isBitSet(disabledPorts, gpios[k])) continue;
      // *LOGGER << "scanning" << endl;
      //if (debug) *LOGGER << "Scanning " << gpios_str[sda], << " : " << gpios_str[sca] << endl;
      if (hasI2CDevices(gpios[i], gpios[k], gpios_str[i], gpios_str[k], debug)) {
        sda = gpios[i];
        scl = gpios[k];
        LOGGER->printf(String(F("Found i2c bus on SDA:SCL = %s:%s (%d:%d)\n")).c_str(), gpios_str[i].c_str(), gpios_str[k].c_str(), sda, scl);
        LOGGER->flush();
        return true;
      }
      delay(1);
    }
  }
  return false;
}

void I2CHelper::i2cHigh() {
  pinMode(i2cSDA, INPUT);
  pinMode(i2cSCL, INPUT);
  delay(1);
}

void I2CHelper::cmdScanI2C(const char *ignore) {
  //if ()
   //pinMode(D8, OUTPUT);    //enable power via D8
  //  digitalWrite(D8, LOW);
  //  delay(500);
//   digitalWrite(D8, HIGH);
//   delay(1000);
  // pinMode(D8, INPUT);
  //brzo_i2c_setup(D5, D1, 50000);
  int8_t a, b;
  findI2C(a, b, 0, true);
}

I2C_STATE I2CHelper::beginI2C(long disabledPorts, uint16_t rtcI2c, Stream *_LOGGER) {
  LOGGER = _LOGGER;
  I2C_STATE ret;
//  brzo_i2c_setup(D5, D1, 50000);
  //Wire.begin(D5, D1);
  //uint32_t rtcI2c = rtcMemStore.getGenData(GEN_I2C_BUS);
  if (rtcI2c) {
    i2cSDA = (int8_t) rtcI2c & 0xFF;
    i2cSCL = (int8_t) ((rtcI2c >> 8) & 0xFF);
  } else if (!findI2C(i2cSDA, i2cSCL, disabledPorts, false)) {
     i2cSDA = i2cSCL = -1;
  }

  if (i2cSDA > -1) {
    String s1 = String(i2cSDA);
    String s2 = String(i2cSCL);
    if (hasI2CDevices(i2cSDA, i2cSCL, s1, s2, false)) {
      *LOGGER << F("Using I2C Bus on SDA:SCA (") << i2cSDA << F(":") << i2cSCL << F(")") << endl;
      Wire.begin(i2cSDA, i2cSCL);
      ret = I2C_BEGIN;
    } else {
      *LOGGER << F("I2C Bus is not responding. NO I2C Devices during this session!")<< endl;
      return I2C_LOST;
      //ESP.restart();
    }
  } else {
    *LOGGER << F("No I2C Devices found\n");
    ret = I2C_NODEVICES;
  }

  // if (!rtcI2c) {
  //   uint32_t data = 0;
  //   data = data | (uint8_t)i2cSDA;
  //   data = data | ((uint8_t)i2cSCL << 8);
  //   rtcMemStore.setGenData(GEN_I2C_BUS, data);
  // }

  return ret;
}

uint8_t I2CHelper::slowEndTransmission(uint8_t sendStop) {
  #ifdef SLOWWIRE
    int8_t ret;
    for (int i=0; i < 5; i++) {
      ret = twi_writeTo(TwoWire::txAddress, TwoWire::txBuffer, TwoWire::txBufferLength, sendStop);
      if (ret == 4) {
        delay(20);
        i2cSlowWireStatus();
        i2cHigh();
        continue;
      }
      break;
    }
    Wire.flush();
    return ret;
  #else
    return Wire.endTransmission(sendStop);
  #endif
}

int I2CHelper::i2cSlowWireStatus() {
  for (int i=0; i < 5 && Wire.status() != I2C_OK; i++) delay(5);
  delay(10);
  return Wire.status();
}

bool I2CHelper::isBitSet(uint32_t val, int bit) {
  return ((val >> bit) &1);
}
