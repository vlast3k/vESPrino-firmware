#include <Wire.h>
#include "sensors\APDS9960Sensor.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"
#include "common.hpp"
#include "lib\SparkFun_APDS9960.h"

int APDS9960Sensor::isr_flag = 0;
APDS9960Sensor::APDS9960Sensor() {
  registerSensor(this);
  APDS9960Sensor::isr_flag = 0;
}

void APDS9960Sensor::setup(MenuHandler *handler) {
  //handler->regsterCommand(new MenuEntry(F("tslInit"), CMD_EXACT, &APDS9960Sensor::onCmdInit, F("")));
  initSensor();
//  closeSensor();
}

void APDS9960Sensor::onCmdInit(const char *ignore) {
  //APDS9960Sensor.initSensor();
}

void APDS9960Sensor::getData(LinkedList<Pair *> *data) {
  // if (!initSensor()) return;
  // data->add(new Pair("LUX", String(tsl->getLuxAutoScale())));
  // closeSensor();
}

bool APDS9960Sensor::initSensor() {
  bool init = false;
  sensor = new SparkFun_APDS9960();
  for (int i=0; i < 5; i++) {
    init = sensor->init();
    if (init) break;
    i2cHigh();
    delay(100);
  }

  if (!init) {
    if (DEBUG) Serial << F("APDS9960 - init failed!") << endl;
    return false;
  }
  SERIAL_PORT << F("Found APDS9960 - Gesture Sensor") << endl;

  pinMode(APDS9960_INT, INPUT);

  // Initialize interrupt service routine
  attachInterrupt(APDS9960_INT, APDS9960Sensor::interruptRoutine, CHANGE);
  // Start running the APDS-9960 gesture sensor engine
  if ( sensor->enableGestureSensor(true) ) {
    Serial.println(F("Gesture sensor is now running"));
  } else {
    Serial.println(F("Something went wrong during gesture sensor init!"));
  }
  return true;
}

void APDS9960Sensor::loop() {
  if( isr_flag == 1 ) {
    detachInterrupt(APDS9960_INT);
    handleGesture();
    isr_flag = 0;
    attachInterrupt(APDS9960_INT, APDS9960Sensor::interruptRoutine, FALLING);
  }
}

void APDS9960Sensor::interruptRoutine() {
  isr_flag = 1;
}

void APDS9960Sensor::handleGesture() {
  if ( sensor->isGestureAvailable() ) {
    char *gesture;
    switch ( sensor->readGesture() ) {
      case DIR_UP:    gesture = "UP";   break;
      case DIR_DOWN:  gesture = "DOWN"; break;
      case DIR_LEFT:  gesture = "LEFT"; break;
      case DIR_RIGHT: gesture = "RIGHT";break;
      case DIR_NEAR:  gesture = "NEAR"; break;
      case DIR_FAR:   gesture = "FAR";  break;
      default:        gesture = "NONE";
    }
//    menuHandler.scheduleCommand("vecmd led_green");
//    menuHandler.scheduleCommand("vecmd ledmode_2_2");
    SERIAL_PORT << "Gesture: " << gesture << endl;
  }
}
