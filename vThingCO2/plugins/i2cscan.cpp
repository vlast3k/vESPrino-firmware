#include <Arduino.h>
#include <Streaming.h>
#include <Wire.h>
#include "common.hpp"

boolean checkI2CDevice(int sda, int sca, int addr) {
  Wire.begin(sda, sca);
  Wire.beginTransmission(addr);
  int res = Wire.endTransmission();
  SERIAL << "i2c addr:" << addr << ", i2c res: " << _HEX(res) << endl;
  return !res;
}

void scani2cBus(int sda, int sca)
{
  Wire.begin(sda, sca);
  byte error, address;
  int nDevices;

  Serial.println("Scanning...");

  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();

    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");

      nDevices++;
    }
    else if (error==4)
    {
      delay(1);
      Serial.print("Unknow error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");

  //delay(5000);           // wait 5 seconds for next scan
}

void scani2c(const char *ignore) {

  Serial <<"I2C Scan SDA : SCA" << endl;
  Serial << "D1 : D5\n";
  scani2cBus(D1, D5);

  Serial << "D5 : D1\n";
  scani2cBus(D5, D1);
  Serial << "D7 : D6\n";
  scani2cBus(D7, D6);
  Serial << "D6 : D7\n";
  scani2cBus(D6, D7);

  Serial << "D5 : D6\n";
  scani2cBus(D5, D6);
  Serial << "D6 : D5\n";
  scani2cBus(D6, D5);
  Serial << "D1 : D6\n";
  scani2cBus(D1, D6);

}
