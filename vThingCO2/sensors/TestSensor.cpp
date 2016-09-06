#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <Wire.h>
#include "sensors\TestSensor.hpp"
#include <LinkedList.h>
#include "interfaces\Pair.h"
#include "common.hpp"

TestSensor::TestSensor() {
  enabled = true;
  registerSensor(this);
}

void TestSensor::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("testSensor"), CMD_EXACT, &TestSensor::toggle, F("testSensor toggle testSesnor")));

}

void TestSensor::toggle(const char *ignore) {
  testSensor.toggleInst();
}

void TestSensor::toggleInst() {
  enabled != enabled;
  Serial << F("Test Sensor:") << (enabled ? F("ENABLED") : F("DISABLED")) << endl;
}

void TestSensor::getData(LinkedList<Pair *> *data) {

  Serial << F("TestSensor::getData") << endl;
  data->add(new Pair("t_TEMP", String(F("23.4"))));
  data->add(new Pair("t_HUM", String(F("65"))));
  data->add(new Pair("t_PRES", String(F("1013"))));
  data->add(new Pair("t_ALT", String(F("560"))));
  data->add(new Pair("t_ALIGHT", String(analogRead(A0))));
}
