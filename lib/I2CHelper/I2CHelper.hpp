#ifndef I2CHelperH
#define I2CHelperH
#include <Wire.h>
#include <Stream.h>
enum I2C_STATE {I2C_BEGIN, I2C_NODEVICES, I2C_LOST};
class I2CHelper {
public:
  static bool checkI2CDevice(int addr);
  static bool checkI2CDevice(int sda, int sca, int addr);
  static void i2cPrintAddr(uint8_t address);
  static void dumpI2CBus(const char *line);
  static bool hasI2CDevices(int sda, int sca, String &sda_str, String &sca_str, bool debug);
  static int i2cWireStatus();
  static int i2cSlowWireStatus();
  static bool findI2C(int8_t &sda, int8_t &scl, long disabledPorts, bool debug);
  static void i2cHigh();
  static void cmdScanI2C(const char *ignore);
  static I2C_STATE beginI2C(long disabledPorts, uint16_t storedI2c, Stream *LOGGER);
  static uint8_t slowEndTransmission(uint8_t sendStop = true);
  static bool isBitSet(uint32_t val, int bit);

  static Stream *LOGGER;

  static int8_t i2cSDA;
  static int8_t i2cSCL;
};

#endif
