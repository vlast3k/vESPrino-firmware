#include "utils\SlowWire.hpp"
#include "common.hpp"
#include "twi.h"



uint8_t SlowWireClass::endTransmission(uint8_t sendStop) {
  int8_t ret;
  for (int i=0; i < 5; i++) {
    ret = twi_writeTo(txAddress, txBuffer, txBufferLength, sendStop);
    //delay(1);
    if (ret == 4) {
      delay(20);
      i2cSlowWireStatus();
      i2cHigh();
      continue;
    }
    break;
  }
  flush();
  return ret;
}
