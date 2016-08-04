#include <Wire.h>
#include <Streaming.h>

bool PM2005_begin(int sda = 0, int sca = 0) {
  if (sda != 0 || sca != 0) Wire.begin(sda, sca);
  Wire.beginTransmission(0x28);
  Wire.write(0x51);
  return Wire.endTransmission(false) == 0;
}

bool PM2005_ReadData(int &pm25, int &pm10, bool debug = false) {
  if (!PM2005_begin()) {
    if (debug) Serial << F("Failed to connect to PM2005\n");
    return false;
  }
  int r;
  byte data[22];
  byte cs = 0;
  r = Wire.requestFrom(0x28, 22, false);
  if (r != 22) {
    if (debug) Serial << "Expected 22 bytes, but got " << r << endl;
    return false;
  }
  for (int i=0; i < 22; i++) {
    data[i] = Wire.read();
    if (debug) Serial << _HEX(data[i]) << F(",");
    if (i < 21) cs ^= data[i];
  }
  if (debug) Serial << endl;
  if (cs != data[21]) {
    if (debug) Serial << F("Wrong Checksum: ") << cs << F(", expected: ") << data[21] << endl;
    return false;
  }
  if (debug) {
    Serial << F("Sensor Status: ") << data[2] << endl;
    Serial << F("PM 2.5 : ") << (data[5] << 2) + data[6] << endl;
    Serial << F("PM  10 : ") << (data[7] << 2) + data[8] << endl;
    Serial << F("Measuring Mode : ") << (data[9] << 2) + data[10] << endl;
  }
  pm25 = (data[5] << 2) + data[6];
  pm10 = (data[7] << 2) + data[8];
  return true;
}
