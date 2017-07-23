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
  bool pms7003_read(int rx, int tx, PMS7003_framestruct &thisFrame);
  void onProperty(String &key, String &value);
  bool hasSensor();
private:
  bool sendCmd(uint8_t *cmd, uint8_t *resp) ;
  void dump1(uint8_t *r, int len = 24);
  void changeMode(int mode);
  void changeSleep(int sleep);
  void doRead() ;
  static void cmd_pms_sleep(const char *line);
  static void cmd_pms_mode(const char *line);
  static void cmd_pms_read(const char *line);
  bool enabled = false;
  int8_t _RX = -2;
  int8_t _TX = -2;
  //uint16_t _RX_TX = 0;
};
#endif
