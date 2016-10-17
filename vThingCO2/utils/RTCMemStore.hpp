#ifndef RTCMemStore_h
#define RTCMemStore_h
#include <RunningAverageSt.h>
struct RTCData {
  uint32_t crc32;
  uint32_t interations = 0;
  uint32_t millisStartIteration;
  uint32_t inDeepSleep = 0;
  RunningAverageSt avg1;
  uint8_t padding[3];
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

  bool wasInDeepSleep();
  void setDeepSleep(bool state);


  static RTCData *rtcData;
  static bool dataExisted;
  static bool getDataExisted();
  static bool test;
   static bool getTest(){return test;};
private:
  static uint32_t calculateCRC32(const uint8_t *data, size_t length);
  static bool readData();
  static void updateData();
};


#endif
