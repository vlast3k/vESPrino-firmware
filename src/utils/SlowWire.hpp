#ifndef SlowWireH
#define SlowWireH
#include <Wire.h>

class SlowWireClass : public TwoWire {
public:
  uint8_t endTransmission(uint8_t sendStop);
  uint8_t endTransmission() {return endTransmission(true);};
};

#endif
