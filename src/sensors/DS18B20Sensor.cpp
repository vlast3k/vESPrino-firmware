#include <Wire.h>
#include "sensors/DS18B20Sensor.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "common.hpp"
#include <OneWire.h>
#include <DallasTemperature.h>

enum DS18B20ReadModes {DS18B20_GETTEMP, DS18B20_ONLY_CHECK};

DS18B20Sensor::DS18B20Sensor() {
  //enabled = true;
  registerSensor(this);
}

void DS18B20Sensor::setup(MenuHandler *handler) {
  // handler->registerCommand(new MenuEntry(F("testSensor"), CMD_EXACT, &DS18B20Sensor::toggle, F("testSensor toggle testSesnor")));
  // enabled = PropertyList.readBoolProperty(F("test.sensor"));

}


// void DS18B20Sensor::toggleInst() {
//   enabled = !enabled;
//   Serial << F("Test Sensor:") << (enabled ? F("ENABLED") : F("DISABLED")) << endl;
//   PropertyList.putProperty(F("test.sensor"), enabled?F("1"): F(""));
// }
//
void DS18B20Sensor::printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}



float getDeviceAddress(DallasTemperature &sensors, DeviceAddress &address) {


}

float DS18B20Sensor::getRawTemperature(int mode) {
  int ONE_WIRE_BUS = D6;
  digitalWrite(ONE_WIRE_BUS, HIGH);
  pinMode(ONE_WIRE_BUS, OUTPUT);
  OneWire oneWire(ONE_WIRE_BUS);
  DallasTemperature sensors(&oneWire);
  DeviceAddress deviceAddress;
  sensors.begin();
  if (!sensors.getAddress(deviceAddress, 0)) {
    Serial.println("Unable to find address for Device 0");
    return 0;
  }
  Serial.print("Device 0 Address: ");
  printAddress(deviceAddress);
  Serial.println();
  if (mode == DS18B20_ONLY_CHECK) return 1;
  sensors.setResolution(deviceAddress, 12);
  sensors.requestTemperatures(); // Send the command to get temperatures
  return sensors.getTempC(deviceAddress);

}

void DS18B20Sensor::getData(LinkedList<Pair *> *data) {
  if (!enabled) return;
  String adj = PropertyList.readProperty(PROP_TEMP_ADJ);
  // double adjTemp = temp + atof(adj.c_str());
  // data->add(new Pair("TEMP", String(adjTemp)));
}
