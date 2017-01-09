#include <Wire.h>
#include "sensors/DS18B20Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "common.hpp"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <plugins/GPIO.hpp>

enum DS18B20ReadModes {DS18B20_GETTEMP, DS18B20_ONLY_CHECK};
#define PROP_DS18B20_PORT F("ds18b20.port")

DS18B20Sensor::DS18B20Sensor() {
  //enabled = true;
  registerSensor(this);
}

bool DS18B20Sensor::setup(MenuHandler *handler) {
  // handler->registerCommand(new MenuEntry(F("testSensor"), CMD_EXACT, &DS18B20Sensor::toggle, F("testSensor toggle testSesnor")));
  // enabled = PropertyList.readBoolProperty(F("test.sensor"));
  char result[10];
  strcpy(result, PropertyList.readProperty(PROP_DS18B20_PORT));
  if (!result[0]) initPort(result);
  if (DEBUG) LOGGER << F("DS18B20 is on: ") << result << endl;
  if (String(result) != F("missing")) {
    hasSensor = true;
    port = GPIOClass::convertToGPIO(result);
    oneWireP = new OneWire(port);
    sensorsP = new DallasTemperature(oneWireP);
    sensorsP->begin();
    getRawTemperature();
  } else {
    hasSensor = false;
  }
  return hasSensor;

}

void DS18B20Sensor::initPort(char *result) {
  char ports[5][4] = {"D1","D5","D6","D7", ""};
  strcpy(result, "missing");
  for (int i=0; ports[i][0]; i++) {
    if (hasSensorOnPort(ports[i])) {
      GPIOClass::reservePort(ports[i]);
      strcpy(result, ports[i]);
      break;
    }
    delay(100);
  }
  PropertyList.putProperty(PROP_DS18B20_PORT, result);
}


// void DS18B20Sensor::toggleInst() {
//   enabled = !enabled;
//   LOGGER << F("Test Sensor:") << (enabled ? F("ENABLED") : F("DISABLED")) << endl;
//   PropertyList.putProperty(F("test.sensor"), enabled?F("1"): F(""));
// }
//
void DS18B20Sensor::printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) LOGGER.print("0");
    LOGGER.print(deviceAddress[i], HEX);
  }
}

bool DS18B20Sensor::hasSensorOnPort(const char *port) {
  if (DEBUG) LOGGER << F("DS18B20: searching on port: ") << port << endl;
  int ONE_WIRE_BUS = GPIOClass::convertToGPIO(port);
  DeviceAddress deviceAddress;
  OneWire oneWire(ONE_WIRE_BUS);
  DallasTemperature sensors(&oneWire);
  sensors.begin();

  if (sensors.getAddress(deviceAddress, 0)) {
    if (DEBUG) {
      LOGGER << F("Found DS18B20 Sensor on port: ") << port << ", internal address: ";
      printAddress(deviceAddress);
      LOGGER << endl;
    }
    return true;
  }
  return false;
}

float DS18B20Sensor::getRawTemperature() {
  int ONE_WIRE_BUS = port;
  pinMode(ONE_WIRE_BUS, OUTPUT);
  digitalWrite(ONE_WIRE_BUS, HIGH);
  //delay(500);
  DeviceAddress deviceAddress;
  if (!sensorsP->getAddress(deviceAddress, 0)) {
    LOGGER.println(F("Unable to find address for Device 0"));
    return 0;
  }
  if (DEBUG) {
    LOGGER.print(F("Device 0 Address: "));
    printAddress(deviceAddress);
    LOGGER.println();
  }
  sensorsP->setResolution(deviceAddress, 12);
  if (DEBUG) LOGGER << F("Request Temp: ") ;
  sensorsP->requestTemperatures(); // Send the command to get temperatures
  if (DEBUG) LOGGER << F("DONE: ") ;
  //delay(1000);
  float f=  sensorsP->getTempC(deviceAddress);
  if (DEBUG) LOGGER << F("Temperature is: ") << f << endl;
  //pinMode(ONE_WIRE_BUS, INPUT);
  return f;


}

void DS18B20Sensor::getData(LinkedList<Pair *> *data) {
  if (!hasSensor) return;
  String adj = PropertyList.readProperty(PROP_TEMP_ADJ);
  float temp = getRawTemperature();
  double adjTemp = temp + atof(adj.c_str());
  String t1 = String(adjTemp);
  String t1r = String(temp);
  replaceDecimalSeparator(t1);
  replaceDecimalSeparator(t1r);
  data->add(new Pair("TEMP", t1));
  data->add(new Pair("TEMPR", t1r));
  // data->add(new Pair("TEMP", String(adjTemp)));
  // data->add(new Pair("TEMPR", String(temp)));
}
