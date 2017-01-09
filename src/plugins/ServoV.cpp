#include "plugins/ServoV.hpp"
#include "MenuHandler.hpp"
#include <Servo.h>

#define PROP_SERVO_PORT F("servo.port")
extern ServoV _ServoV;

ServoV::ServoV() {
  registerPlugin(this);
}

bool ServoV::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("setServoPort"), CMD_BEGIN, ServoV::cmdSetServoPort, F("setServoPort [0:16]? (blank for off)")));
  handler->registerCommand(new MenuEntry(F("setServoPos"), CMD_BEGIN, ServoV::cmdSetServoPos, F("setServoPos [0:180]")));
  //s.attach(D6);
  initServo();
  return false;

}

void ServoV::cmdSetServoPort(const char *cmd) {
  cmd = strchr(cmd, ' ');
  if (cmd == NULL || cmd[1] == 0) {
    PropertyList.putProperty(PROP_SERVO_PORT, "");
  } else {
    PropertyList.putProperty(PROP_SERVO_PORT, cmd + 1);
  }
}

void ServoV::cmdSetServoPos(const char *cmd) {
   _ServoV.cmdSetServoPosInst(cmd);
}

void ServoV::cmdSetServoPosInst(const char *cmd) {
  cmd = strchr(cmd, ' ');
  if (cmd != NULL) {
    int pos = atoi(cmd+1);
    s.write(pos);
    LOGGER << F("Move Servo to: ") << pos << endl;
    delay(500);
  }
}

void ServoV::initServo() {
  String sport = PropertyList.readProperty(PROP_SERVO_PORT);
  if (sport.length() == 0) {
    s.detach();
    // if (servo) {
    //   servo->detach();
    //   delete servo;
    //   servo = NULL;
    // }
  } else {
    int port = atoi(sport.c_str());
    // if (servo == NULL) servo = new Servo();
    //servo->detach();
    s.detach();
    LOGGER << F("Attach Servo to: ") << port << endl;
    #ifndef ESPSTAGE
    s.attach(port);
    #endif
    //servo->attach(port);
  }
}
