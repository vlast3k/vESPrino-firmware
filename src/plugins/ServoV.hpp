#ifndef ServoV_H
#define ServoV_H
#include "common.hpp"
#include "PropertyList.hpp"
#include "interfaces/Plugin.hpp"
#include <Servo.h>

class ServoV : public Plugin {
public:
  ServoV() ;
  bool setup(MenuHandler *handler);
  const char* getName() {
    return "Servo";
  }
  static void cmdSetServoPort(const char *cmd);
  static void cmdSetServoPos(const char *cmd);
  void cmdSetServoPosInst(const char *cmd);
  void initServo();
  //Servo *servo = NULL;
  Servo s;
};
#endif
