#include <Arduino.h>

#include <Wire.h>

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

void scani2c() {
  
  Serial.println("\nI2C Scanner D1 D6");
  scani2cBus(D1, D6);

  Serial.println("\nI2C Scanner D6 D1");
  scani2cBus(D6, D1);

  Serial.println("\nI2C Scanner D7 D5");
  scani2cBus(D7, D5);

  Serial.println("\nI2C Scanner D5 D7");
  scani2cBus(D5, D7);

}

