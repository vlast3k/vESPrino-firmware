//#ifdef VTHING_CO2

#include "Arduino.h"
#include "Timer.h"
#include "CubicGasSensors.h"
#include "common.hpp"
#include "plugins/PropertyList.hpp"
#include "plugins/GPIO.hpp"
#include "EEPROM.h"
#include "CO2Sensor.hpp"
#include <LinkedList.h>
#include <I2CHelper.hpp>
#include "plugins/TimerManager.hpp"
extern TimerManagerClass TimerManager;

extern CO2Sensor co2Sensor;

#define CUBIC_PRESENT "present"
#define CUBIC_MISSING "missing"
extern TimerManagerClass TimerManager;
CO2Sensor::CO2Sensor() :
    co2Threshold("CO2", 1),
    cubicCo2(&CO2Sensor::onCo2Status_static, EE_RESET_CO2_1B, &LOGGER),
    startedCO2Monitoring(false)

{
  registerSensor(this);
}


bool CO2Sensor::setup(MenuHandler *handler) {
  if (!rtcMemStore.hasSensor(RTC_SENSOR_CUBICCO2)) {
    hasSensor = false;
    return hasSensor;
  }
  String state = PropertyList.readProperty(PROP_NO_CUBIC_CO2);
  if (state == CUBIC_MISSING) {
    hasSensor = false;
    rtcMemStore.setSensorState(RTC_SENSOR_CUBICCO2, false);
    return hasSensor;
  }
  // pinMode(D8, OUTPUT);    //enable power via D8
  // digitalWrite(D8, HIGH);
  // delay(1000);
  //this is needed as if the LOGGER Port querries the port of the servo, then servo no longer wifiScanNetworks
  //maybe due to the timers...
  String servoPort = PropertyList.readProperty(PROP_SERVO_PORT);
  uint32_t disabledSerialPorts = PropertyList.readLongProperty(PROP_I2C_DISABLED_PORTS);
  GPIOClass::setBit(disabledSerialPorts, I2CHelper::i2cSDA, 1);
  GPIOClass::setBit(disabledSerialPorts, I2CHelper::i2cSCL, 1);
  if (servoPort.length()) GPIOClass::setBit(disabledSerialPorts, atoi(servoPort.c_str()), 1);
  if (!cubicCo2.init(DEBUG, disabledSerialPorts)) {
    hasSensor = false;
    rtcMemStore.setSensorState(RTC_SENSOR_CUBICCO2, false);
    if (state != CUBIC_PRESENT) PropertyList.putProperty(PROP_NO_CUBIC_CO2, CUBIC_MISSING);
    return hasSensor;
  }

  if (state != CUBIC_PRESENT) {
    PropertyList.putProperty(PROP_NO_CUBIC_CO2, CUBIC_PRESENT);
  }
//  cubicCo2.init();// return;
  hasSensor = true;

  tmrStopLED = new Timer(30000L, CO2Sensor::onStopLED_static, true);
  TimerManager.registerTimer(tmrStopLED, TMR_STOPPED);

  thresholds.add(&co2Threshold);


  handler->registerCommand(new MenuEntry(F("rco"), CMD_EXACT, CO2Sensor::resetCO2_static, F("")));

  LOGGER << F("CO2 now: ") << cubicCo2.rawReadCM1106_CO2() << endl;
  if (!PropertyList.readBoolProperty(PROP_CUBIC_CO2_POWERSAFE)) {
    menuHandler.scheduleCommand("nop 0");
  }
  return hasSensor;
}

const char* CO2Sensor::getSensorId() {
  static const char id[] = "CO2";
  return id;
}

float CO2Sensor::getValue() {
  int res = cubicCo2.getCO2(DEBUG);
  startedCO2Monitoring = cubicCo2.hasStarted();
  //LOGGER << " CO2 Sensor > startedCO2Mon = " << startedCO2Monitoring << " co2=" << res << endl;
  if (startedCO2Monitoring) {
    return res;
  } else {
    return -1;
  }
}

void CO2Sensor::getData(LinkedList<Pair*> *data) {
  if (!hasSensor) return;
//  Pair *p = new Pair(getSensorId(), String(getValue()));
  //p->key = getSensorId();
  //p->value = String(getValue());
  float f = getValue();
  if (f > 0) data->add(new Pair(getSensorId(), String(f)));
}

void CO2Sensor::onStopLED_static() {
  co2Sensor.onStopLED();
}

void CO2Sensor::onStopLED() {
  menuHandler.scheduleCommand("ledbrg 98");
  menuHandler.scheduleCommand("ledcolor black");
    // strip->SetPixelColor(0, RgbColor(0, 0,0));
    // strip->Show();
}

void CO2Sensor::onCo2Status_static(CubicStatus status) {
  co2Sensor.onCo2Status(status);
}

void CO2Sensor::onCo2Status(CubicStatus status) {
  if (status == CB_CALIBRATE) {
    menuHandler.scheduleCommand("nop 0");
    menuHandler.scheduleCommand("ledcolor blue");
    tmrStopLED->Stop();
  } else if (status == CB_WARMUP) {
    //menuHandler.scheduleCommand("ledcolor yellow");
  } else if (status == CB_STARTED) {
    if (!PropertyList.readBoolProperty(PROP_CUBIC_CO2_POWERSAFE)) {
      //menuHandler.scheduleCommand("ledcolor green");
      //tmrStopLED->Start();
    }
  }
}

void CO2Sensor::resetCO2_static(const char *ignore) {
  co2Sensor.resetCO2();
}

void CO2Sensor::resetCO2() {
  LOGGER << F("Calibration Mode Enabled.\nPlease put the device for 8 minutes at fresh air.\nYou can now put it outside. It will complete calibration once it worked 5 minutes after restart") << endl;
  LOGGER.flush();
  EEPROM.begin(10);
  EEPROM.put(EE_RESET_CO2_1B, (byte)1);
  EEPROM.commit();
  ESP.restart();
}
