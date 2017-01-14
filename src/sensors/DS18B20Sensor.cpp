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
#define PROP_DS18B20_DEVICE_LIST F("ds18b20.devlist")
#define PROP_DS18B20_ADJ F("ds18b20.adj")
extern DS18B20Sensor _DS18B20Sensor;

DS18B20Sensor::DS18B20Sensor() {
  //enabled = true;
  registerSensor(this);
}

bool DS18B20Sensor::setup(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("dsScan"), CMD_EXACT, &DS18B20Sensor::scanBus, F("testSensor toggle testSesnor")));
  handler->registerCommand(new MenuEntry(F("dsAllTemp"), CMD_EXACT, &DS18B20Sensor::readTemperatures, F("testSensor toggle testSesnor")));
  handler->registerCommand(new MenuEntry(F("dsAdj"), CMD_EXACT, &DS18B20Sensor::computeTempAdjustments, F("testSensor toggle testSesnor")));
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

void DS18B20Sensor::scanBus(const char* ignore) {
  _DS18B20Sensor.scanBusInst();
}
void DS18B20Sensor::scanBusInst() {
  int ONE_WIRE_BUS = port;
  pinMode(ONE_WIRE_BUS, OUTPUT);
  digitalWrite(ONE_WIRE_BUS, HIGH);
  String oldDevices = PropertyList.readProperty(PROP_DS18B20_DEVICE_LIST);
  for (int i=0; i < sensorsP->getDeviceCount(); i++) {
    DeviceAddress deviceAddress;
    if (!sensorsP->getAddress(deviceAddress, i)) {
      LOGGER << F("Unable to find address for Device ") << i << endl;
      return;
    }
    char addrStr[17];
    for (int i=0; i < 8; i++) sprintf(&addrStr[i*2], "%02X", deviceAddress[i]);
    addrStr[16] = 0;
    bool isNew = oldDevices.indexOf(addrStr) == -1;
    Serial << i << " : " << addrStr << " : " << isNew << endl;
    if (isNew) {
      if (oldDevices.length()) oldDevices += ",";
       oldDevices += addrStr;
    }
  }
  PropertyList.putProperty(PROP_DS18B20_DEVICE_LIST, oldDevices.c_str());
}

void DS18B20Sensor::readTemperatures(const char *ignore) {
  _DS18B20Sensor.readTemperaturesInst();
}

void DS18B20Sensor::readTemperaturesInst(LinkedList<float> *list) {
  int ONE_WIRE_BUS = port;
  pinMode(ONE_WIRE_BUS, OUTPUT);
  digitalWrite(ONE_WIRE_BUS, HIGH);
  sensorsP->setResolution(12);
  sensorsP->requestTemperatures(); // Send the command to get temperatures
  String oldDevices = PropertyList.readProperty(PROP_DS18B20_DEVICE_LIST);
  int savedCount = getListItemCount(oldDevices.c_str());
  for (int i=0; i<savedCount; i++) {
    char addr[20];
    getListItem(oldDevices.c_str(), addr, i);
    DeviceAddress devAddr;
    char toConv[3];
    //Serial << "Converting: " << addr << endl;
    for (int i=0; i < 8; i++) {
      toConv[0] = addr[i*2];
      toConv[1] = addr[i*2 + 1];
      toConv[2] = 0;
      devAddr[i] = strtoul(toConv, NULL, 16);
      //Serial << _HEX(devAddr[i]) << ",";
    }
    //Serial << endl;
    float f=  sensorsP->getTempC(devAddr);
    Serial << i << " : " << addr << " : " << f << endl;
    if (list) list->add(f);
  }
}

double DS18B20Sensor::getAverageTemperature(LinkedList<float> *list) {
  double sum = 0;
  for (int i=0; i < list->size(); i++) sum += list->get(i);
  return sum / list->size();
}

void DS18B20Sensor::computeTempAdjustments(const char* ignore) {
  _DS18B20Sensor.computeTempAdjustmentsInst();
}
void DS18B20Sensor::computeTempAdjustmentsInst() {
  LinkedList<float> *list = new LinkedList<float>();
  readTemperaturesInst(list);
  double avg = getAverageTemperature(list);
  String adjustments;
  for (int i=0; i < list->size(); i++) adjustments += String(list->get(i) - avg) + ",";
  //Serial << "adj before: " << adjustments << endl;
  adjustments = adjustments.substring(0, adjustments.length() - 1);
  //Serial << "adj after: " << adjustments << endl;
  PropertyList.putProperty(PROP_DS18B20_ADJ, adjustments.c_str());
  delete list;
}

bool DS18B20Sensor::readAdjustedTemperatures(LinkedList<float> *list) {
  readTemperaturesInst(list);
  String adjustments = PropertyList.readProperty(PROP_DS18B20_ADJ);
  int count = getListItemCount(adjustments.c_str());
  //LOGGER.printf(String(F("Found %d saved addresses and %d adjustments. They must be equal.\n")).c_str(), list->size(), count);
  if (count != list->size()) {
    LOGGER.printf(String(F("Found %d saved addresses and %d adjustments. They must be equal.\n")).c_str(), list->size(), count);
    return false;
  }
  float base = 1000;// list->get(0);
  for (int i=0; i < count; i++) {
    char buf[20];
    float adj = atof(getListItem(adjustments.c_str(), buf, i));
    float adjTemp = list->get(i) - adj;
    if (base > 900) base = adjTemp;
    LOGGER << i << ":" << list->get(i) << " | " << adj << " | " <<  adjTemp << " | " << adjTemp - base << endl;
//    LOGGER.printf("%d: %s | %s | %s\n", i, String(list->get(i)).c_str(), String(adj), String(list->get(i) - adj));
    list->set(i, list->get(i) - adj);
  }
  return true;
}

void DS18B20Sensor::appendTemperatures(LinkedList<Pair *> *data) {
  LinkedList<float> temps = LinkedList<float>();
  if (!readAdjustedTemperatures(&temps)) return;
  for (int i=0; i < temps.size(); i++) {
    String sa = "TEMP";
    sa += String(i);
    data->add(new Pair(sa.c_str(), String(temps.get(i))));
  }
  return;
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
  appendTemperatures(data);
  // data->add(new Pair("TEMP", String(adjTemp)));
  // data->add(new Pair("TEMPR", String(temp)));
}
