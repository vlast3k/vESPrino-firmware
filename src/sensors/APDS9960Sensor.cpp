#include <Wire.h>
#include "sensors/APDS9960Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "common.hpp"
#include "SparkFun_APDS9960.h"

//int APDS9960Sensor::isr_flag = 0;
APDS9960Sensor::APDS9960Sensor() {
  registerSensor(this);
//  APDS9960Sensor::isr_flag = 0;
}

bool APDS9960Sensor::setup(MenuHandler *handler) {
  if (!initSensor()) {
    closeSensor();
    return false;
  }
  return true;
}

void APDS9960Sensor::onCmdInit(const char *ignore) {
}

void APDS9960Sensor::getData(LinkedList<Pair *> *data) {
}

bool APDS9960Sensor::initSensor() {
  if (!rtcMemStore.hasSensor(RTC_SENSOR_APDS9960)) return false;
  bool init = false;
  pinMode(D8, OUTPUT);
  bool d8prev = digitalRead(D8);
  if (!d8prev) digitalWrite(D8, HIGH);
  sensor = new SparkFun_APDS9960();
  for (int i=0; i < 5; i++) {
    init = sensor->init();
    if (init) break;
    I2CHelper::i2cHigh();
    delay(10);
  }

  if (!init) {
    //if (DEBUG) LOGGER << F("APDS9960 - init failed!") << endl;
    digitalWrite(D8, d8prev);
    rtcMemStore.setSensorState(RTC_SENSOR_APDS9960, false);
    return false;
  }
  //LOGGER << F("Found APDS9960 - Gesture Sensor") << endl;

  if ( sensor->enableGestureSensor(true) ) {
    LOGGER.println(F("Gesture sensor is now running"));
  } else {
    LOGGER.println(F("Something went wrong during gesture sensor init!"));
  }
  menuHandler.scheduleCommand("nop 0");
  return true;
}

void APDS9960Sensor::loop() {
    handleGesture();
}

void APDS9960Sensor::handleGesture() {
  if (!sensor) return;
  if (sensor->isGestureAvailable()) {
    String gesture;
    switch ( sensor->readGesture() ) {
      case DIR_UP:    gesture = F("UP");   break;
      case DIR_DOWN:  gesture = F("DOWN"); break;
      case DIR_LEFT:  gesture = F("LEFT"); break;
      case DIR_RIGHT: gesture = F("RIGHT");break;
      case DIR_NEAR:  gesture = F("NEAR"); break;
      case DIR_FAR:   gesture = F("FAR");  break;
      default:        gesture = F("NONE");
    }
//    menuHandler.scheduleCommand("vecmd led_green");
//    menuHandler.scheduleCommand("vecmd ledmode_2_2");
    LOGGER << F("Gesture: ") << gesture << endl;
    String cmd = PropertyList.readProperty(PROP_ONGESTURE);
    if (cmd.length()) {
      cmd.replace("%GESTURE%", gesture);
      menuHandler.scheduleCommand(cmd.c_str());
    }
    String event = "onGesture_";
    event += gesture;
    fireEvent(event.c_str());
  }
}

void APDS9960Sensor::closeSensor() {
  if (!sensor) return;
  delete sensor;
  sensor = NULL;
}
