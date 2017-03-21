#include <Wire.h>
#include "sensors/TGS8100.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "common.hpp"
#include <I2CHelper.hpp>

extern int16_t globalTemp;
extern int8_t  globalHum;

#define MSG_SET_TEMP_HUM 0x1
#define MSG_RESET 0x2

TGS8100::TGS8100() {
  //enabled = true;
  registerSensor(this);
}

bool TGS8100::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("tg8test"), CMD_EXACT, &TGS8100::test, F("TGS8100 toggle testSesnor")));
  handler->registerCommand(new MenuEntry(F("tg8rst"), CMD_EXACT, &TGS8100::reset, F("TGS8100 reset")));
//  enabled = PropertyList.readBoolProperty(F("test.sensor"));
  if (I2CHelper::i2cSDA != -1 && I2CHelper::checkI2CDevice(0x8)) enabled = true;
  if (!enabled) {
    for (int i=0; i < 100; i++) {
      if (enabled = I2CHelper::checkI2CDevice(0x8)) break;
    }
  }
  return enabled;
}

void TGS8100::reset(const char *ignore) {
  Wire.setClock(10000L);
  //bool x = I2CHelper::checkI2CDevice(0x8);
  Wire.beginTransmission(0x8);
  Wire.endTransmission();
  delay(10);
  Wire.beginTransmission(0x8);
  Wire.write(MSG_RESET);
  Wire.write(42);
  Wire.endTransmission();
}

int TGS8100::readSensorValue(uint16_t &raw, uint16_t &rs, double &ppm, uint16_t &rsAdj, uint16_t &maxR0) {
  Wire.setClock(10000L);
  //bool x = I2CHelper::checkI2CDevice(0x8);
  Wire.beginTransmission(0x8);
  Wire.endTransmission();
  delay(10);
  Wire.beginTransmission(0x8);
  Wire.write(MSG_SET_TEMP_HUM);
  Wire.write((uint8_t) (globalTemp >> 8));
  Wire.write((uint8_t) (globalTemp & 0xFF));
  Wire.write((uint8_t) (globalHum & 0xFF));
  Wire.write(42);
  Wire.endTransmission();
  //delay(50);
  //Serial << "tgs811 state: " << x << endl;
  delay(10);
  int len = Wire.requestFrom((uint8_t)8, (size_t)15, true);    // request 6 bytes from slave device #8
  Serial << "Received " << len << " bytes\n";
  uint8_t rcv[20];
  int i = 0;
  for (int i=0; i < len; i++) {
    rcv[i] = Wire.read();
    Serial << _HEX(rcv[i]) << ",";
  }
  Serial << endl;
  if (rcv[12] != 42) {
    Serial << "TGS8100: ERROR in i2c Communication" << endl;
    return 0;
  }
      raw = (rcv[0] << 8) + rcv[1];
  rs      = (rcv[2] << 8) + rcv[3];
  uint16_t ppm100 = (rcv[4] << 8) + rcv[5];
  rsAdj  = (rcv[6] << 8) + rcv[7];
  uint16_t vcc    = (rcv[8] << 8) + rcv[9];
  maxR0 = (rcv[10] << 8) + rcv[11];
  ppm = (float)ppm100/100;
  return raw;

}

extern TGS8100 _TGS8100;
void TGS8100::test(const char *ignore) {
  uint16_t raw, rs, rsa, r0;
  double ppm;
  Serial << _TGS8100.readSensorValue(raw, rs, ppm, rsa, r0) << endl;

  // for (int i=0; i < 100; i++) {
  //   Serial << i << ": " << _TGS8100.readSensorValue() << endl;
  //   delay(5000);
  // }
}

void TGS8100::getData(LinkedList<Pair *> *data) {
  if (!enabled) return;

  LOGGER << F("TGS8100::getData") << endl;
  uint16_t raw, rs, rsa, r0;
  double ppm;
  int x = readSensorValue(raw, rs, ppm, rsa, r0);
  if (x > 0) {
    data->add(new Pair("VOC_RAW", String(raw)));
    data->add(new Pair("VOC_RS", String(rs)));
    data->add(new Pair("VOC_PPM", String(ppm)));
    data->add(new Pair("VOC_APPM", String(ppm*400)));
    data->add(new Pair("VOC_RSA", String(rsa)));
    data->add(new Pair("VOC_R0", String(r0)));
  }
}
