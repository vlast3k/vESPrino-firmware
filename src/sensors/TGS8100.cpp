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
  //handler->registerCommand(new MenuEntry(F("TGS8100"), CMD_EXACT, &TGS8100::toggle, F("TGS8100 toggle testSesnor")));
//  enabled = PropertyList.readBoolProperty(F("test.sensor"));
  if (I2CHelper::i2cSDA != -1 && I2CHelper::checkI2CDevice(0x8)) enabled = true;
  if (!enabled) {
    for (int i=0; i < 100; i++) {
      if (enabled = I2CHelper::checkI2CDevice(0x8)) break;
    }
  }
  return enabled;
}


void TGS8100::getData(LinkedList<Pair *> *data) {
  if (!enabled) return;
  LOGGER << F("TGS8100::getData") << endl;
  Wire.requestFrom((uint8_t)8, (size_t)3, true);    // request 6 bytes from slave device #8
  uint8_t rcv[5];
  int i = 0;
  for (int i=0; i < 3; i++) rcv[i] = Wire.read();
  if (rcv[2] != 42) {
    Serial << "TGS8100: ERROR in i2c Communication" << endl;
    return;
  }
  int value = (rcv[0] << 8) + rcv[1];
  data->add(new Pair("VOC", String(value)));
}
