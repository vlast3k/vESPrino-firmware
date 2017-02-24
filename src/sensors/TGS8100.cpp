#include <Wire.h>
#include "sensors/TGS8100.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "common.hpp"
#include <I2CHelper.hpp>

TGS8100::TGS8100() {
  //enabled = true;
  registerSensor(this);
}

bool TGS8100::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("tg8test"), CMD_EXACT, &TGS8100::test, F("TGS8100 toggle testSesnor")));
//  enabled = PropertyList.readBoolProperty(F("test.sensor"));
  if (I2CHelper::i2cSDA != -1 && I2CHelper::checkI2CDevice(0x8)) enabled = true;
  if (!enabled) {
    for (int i=0; i < 100; i++) {
      if (enabled = I2CHelper::checkI2CDevice(0x8)) break;
    }
  }
  return enabled;
}

int TGS8100::readSensorValue(uint16_t &raw, uint16_t &rs, double &ppm) {
  Wire.setClock(10000L);
  int len = Wire.requestFrom((uint8_t)8, (size_t)7, true);    // request 6 bytes from slave device #8
  Serial << "Received " << len << " bytes\n";
  uint8_t rcv[7];
  int i = 0;
  for (int i=0; i < 7; i++) {
    rcv[i] = Wire.read();
    Serial << _HEX(rcv[i]) << ",";
  }
  Serial << endl;
  if (rcv[6] != 42) {
    Serial << "TGS8100: ERROR in i2c Communication" << endl;
    return 0;
  }
      raw = (rcv[0] << 8) + rcv[1];
  rs      = (rcv[2] << 8) + rcv[3];
  uint16_t ppm100 = (rcv[4] << 8) + rcv[5];
  ppm = (float)ppm100/100;
  return raw;

}

extern TGS8100 _TGS8100;
void TGS8100::test(const char *ignore) {
  for (int i=0; i < 100; i++) {
    //Serial << i << ": " << _TGS8100.readSensorValue() << endl;
    delay(5000);
  }
}

void TGS8100::getData(LinkedList<Pair *> *data) {
  if (!enabled) return;
  LOGGER << F("TGS8100::getData") << endl;
  uint16_t raw, rs;
  double ppm;
  int x = readSensorValue(raw, rs, ppm);
  if (x > 0) {
    data->add(new Pair("VOC_RAW", String(raw)));
    data->add(new Pair("VOC_RS", String(rs)));
    data->add(new Pair("VOC_PPM", String(ppm)));
  }
}
