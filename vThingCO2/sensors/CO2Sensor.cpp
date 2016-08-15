//#ifdef VTHING_CO2

#include "Arduino.h"
#include "Timer.h"
#include "CubicGasSensors.h"
#include "common.hpp"
#include "plugins\PropertyList.hpp"
#include "EEPROM.h"
#include "CO2Sensor.hpp"
#include <LinkedList.h>

extern CO2Sensor co2Sensor;

CO2Sensor::CO2Sensor() :
    co2Threshold("CO2", 1),
    cubicCo2(&CO2Sensor::onCo2Status_static, EE_RESET_CO2_1B),
    startedCO2Monitoring(false)
{
  sensors.add(this);
}


void CO2Sensor::setup(MenuHandler *handler) {
  cubicCo2.init();

  tmrStopLED = new Timer(30000L, CO2Sensor::onStopLED_static, true);
  timers.add(tmrStopLED);

  thresholds.add(&co2Threshold);


  handler->registerCommand(new MenuEntry(F("rco"), CMD_EXACT, CO2Sensor::resetCO2_static, F("")));

  SERIAL << F("CO2 now: ") << cubicCo2.rawReadCM1106_CO2() << endl;
}

const char* CO2Sensor::getSensorId() {
  static const char id[] = "CO2";
  return id;
}

float CO2Sensor::getValue() {
  int res = cubicCo2.getCO2(DEBUG);
  startedCO2Monitoring = cubicCo2.hasStarted();
  if (startedCO2Monitoring) {
    return res;
  } else {
    return nanf("");
  }
}

void CO2Sensor::getData(LinkedList<Pair*> *data) {
  Pair *p = new Pair();
  p->key = getSensorId();
  p->value = String(getValue());
  data->add(p);
}

void CO2Sensor::onStopLED_static() {
  co2Sensor.onStopLED();
}

void CO2Sensor::onStopLED() {
    strip->SetPixelColor(0, RgbColor(0, 0,0));
    strip->Show();
}

void CO2Sensor::onCo2Status_static(CubicStatus status) {
  co2Sensor.onCo2Status(status);
}

void CO2Sensor::onCo2Status(CubicStatus status) {
  //SERIAL << "status = " << status << endl;
  if (status == CB_CALIBRATE) {
   // SERIAL << "status11111 = " << status << endl;
    strip->SetPixelColor(0, RgbColor(0, 0,30));
    tmrStopLED->Stop();
  } else if (status == CB_WARMUP) {
    strip->SetPixelColor(0, RgbColor(30, 30, 0));
  } else if (status == CB_STARTED) {
    strip->SetPixelColor(0, RgbColor(0, 30, 0));
    tmrStopLED->Start();
  }
  strip->Show();
}

void CO2Sensor::resetCO2_static(const char *ignore) {
  co2Sensor.resetCO2();
}

void CO2Sensor::resetCO2() {
  SERIAL << F("Calibration Mode Enabled.\nPlease put the device for 5 minutes at fresh air.\nYou can now put it outside. It will complete calibration once it worked 5 minutes after restart") << endl;
  EEPROM.begin(10);
  EEPROM.put(EE_RESET_CO2_1B, (byte)1);
  EEPROM.commit();
  ESP.restart();
}
