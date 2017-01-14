#ifndef RTCMemStore_h
#define RTCMemStore_h
#include <RunningAverageSt.h>

#define GEN_MSCOUNTER 0
#define GEN_LASTTIME 1
#define GEN_SENSOR_STATE 2
#define GEN_I2C_BUS 3
#define GEN_WIFI_STATIC_IP 4
#define GEN_WIFI_GW 5
#define GEN_WIFI_SUB 6
#define GEN_WIFI_DNS1 7
#define GEN_WIFI_DNS2 8
#define GEN_TIME_LAST_UPD_INT 9

#define RTC_SENSOR_APDS9960 0
#define RTC_SENSOR_BH1750   1
#define RTC_SENSOR_BME280   2
#define RTC_SENSOR_BMP180   3
#define RTC_SENSOR_CDM7160  4
#define RTC_SENSOR_CUBICCO2 5
#define RTC_SENSOR_PM2005   6
#define RTC_SENSOR_SI7021   7
#define RTC_SENSOR_TSL2561  8

struct RTCData {
  RTCData() {
    genData[GEN_MSCOUNTER] = 0;
    genData[GEN_SENSOR_STATE] = 0xFFFF;
    genData[GEN_I2C_BUS] = 0;
    genData[GEN_LASTTIME] = 0;
    genData[GEN_WIFI_STATIC_IP] = 0;
    genData[GEN_WIFI_GW] = 0;
    genData[GEN_WIFI_SUB] = 0;
    genData[GEN_WIFI_DNS1] = 0;
    genData[GEN_WIFI_DNS2] = 0;
    genData[GEN_TIME_LAST_UPD_INT] = 0;
    lastDweetCreated[0] = 0;
  }
  uint32_t crc32;
  uint32_t interations = 0;
  uint32_t millisStartIteration;
  uint32_t inDeepSleep = 0;
  uint32_t genData[10];
  RunningAverageSt avg1;
  uint8_t padding[3];
  char lastDweetCreated[30];
  static int rtcDataSize() {
    return (sizeof(struct RTCData)/4) * 4;
  }
};
// struct RTCData2 {
//   uint32_t crc32;
//   uint8_t data[508];
// };

class RTCMemStore {
public:
  void init();
  uint8_t getIterations();
  void setIterations(uint8_t i);
  double getAverage();
  void addAverageValue(double val, int weight = 1);
  static uint32_t getMillis();
  void addMillis(uint32_t ms);
  static uint32_t getGenData(int idx);
  static void setGenData(int idx, uint32_t value);
  bool wasInDeepSleep();
  void setDeepSleep(bool state);
  void clear();
  void setSensorState(int sensorIdx, bool state);
  bool hasSensor(int sensorIdx);

  void setLastDweet(const char *s);
  void getLastDweet(char *buf);

  static RTCData *rtcData;
  static bool dataExisted;
  static bool getDataExisted();
  static bool test;
  static bool getTest() { return test; };
private:
  static uint32_t calculateCRC32(const uint8_t *data, size_t length);
  static bool readData();
  static void updateData();
  static void setBit(uint32_t &val, int bit, int state);
  static bool isBitSet(uint32_t val, int bit);

};


#endif
