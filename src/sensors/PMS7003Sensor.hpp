#ifndef PMS7003Sensor_h
#define PMS7003Sensor_h

#include "interfaces/Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#define  MAX_FRAME_LEN 64
struct PMS7003_framestruct {
    uint8_t  frameHeader[2];
    uint16_t frameLen = MAX_FRAME_LEN;
    uint16_t concPM1_0_CF1;
    uint16_t concPM2_5_CF1;
    uint16_t concPM10_0_CF1;
    uint16_t concPM1_0_amb;
    uint16_t concPM2_5_amb;
    uint16_t concPM10_0_amb;
    uint16_t rawGt0_3um;
    uint16_t rawGt0_5um;
    uint16_t rawGt1_0um;
    uint16_t rawGt2_5um;
    uint16_t rawGt5_0um;
    uint16_t rawGt10_0um;
    uint8_t  version;
    uint8_t  errorCode;
    uint16_t checksum;
};

class PMS7003Sensor : public Sensor {
public:
  PMS7003Sensor();
  bool setup(MenuHandler *handler);
  void getData(LinkedList<Pair*> *data);
  const char* getName() {
    return "PMS7003";
  }
  static void test(const char *ig);
  bool pms7003_read(PMS7003_framestruct &thisFrame);
  void onProperty(String &key, String &value);
private:
  bool enabled = false;
};
#endif
