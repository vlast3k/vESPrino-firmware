#include <Arduino.h>
#include "MenuHandler.hpp"
#include "Timer.h"
#include "LinkedList.h"
#include "interfaces/Destination.hpp"
#include "interfaces/Plugin.hpp"
#include "interfaces/Sensor.hpp"
#include "interfaces/Pair.h"
#include "sensors/CO2Sensor.hpp"
#include "destinations/CustomHTTPDest.hpp"
#include "plugins/PropertyList.hpp"
#include "plugins/TimerManager.hpp"
#include "plugins/PowerManager.hpp"
#include "sensors/SI7021Sensor.hpp"
#include "sensors/BME280Sensor.hpp"
#include "sensors/BMP085Sensor.hpp"
#include "sensors/PM2005Sensor.hpp"
#include "sensors/CDM7160Sensor.hpp"
#include "sensors/LDR.hpp"
#include "sensors/TestSensor.hpp"
#include "sensors/TSL2561Sensor.hpp"
#include "sensors/BH1750Sensor.hpp"
#include "sensors/APDS9960Sensor.hpp"
#include "destinations/MQTTDest.hpp"
#include "destinations/SerialDumpDest.hpp"
#include "destinations/RFDest.hpp"
#include <RTCMemStore.hpp>
#include "plugins/NeopixelVE.hpp"
#include "plugins/Dweet_io.hpp"
#include "plugins/WebServer.hpp"
#include "plugins/WebSocketServer.hpp"
#include "plugins/Button.hpp"
#include "plugins/GPIO.hpp"
#include "plugins/ServoV.hpp"
#include "plugins/Annemometer.hpp"
#include "plugins/DestinationManager.hpp"
#include "destinations/BlynkDest.hpp"
#include "sensors/DS18B20Sensor.hpp"
#include "plugins/LoggingPrinter.hpp"
#include "plugins/WifiStuff.hpp"


//boolean hasSSD1306 = false, hasSI7021 = false, hasPN532=false, hasBMP180=false, hasBH1750=false, hasAPDS9960=false;
//boolean hasPIR = false;
bool shouldSend = false;
boolean DEBUG = false;
boolean SKIP_LOOP = false;
MenuHandler menuHandler;
char atCIPSTART_IP[20];
char commonBuffer200[200];
bool deepSleepWake = false;
String storedSensors;
//int8_t i2cSDA = -1, i2cSCL = -1;

LinkedList<Plugin *> plugins = LinkedList<Plugin *>();
LinkedList<Sensor *> sensors = LinkedList<Sensor *>();
LinkedList<Destination *> destinations = LinkedList<Destination *>();
LinkedList<Thr *> thresholds = LinkedList<Thr *>();
//LinkedList<Timer *> timers = LinkedList<Timer *>();
WifiStuffClass WifiStuff;
WebSocketServerClass myWSS;
PropertyListClass PropertyList;
PowerManagerClass PowerManager;
TimerManagerClass TimerManager;
RTCMemStore rtcMemStore;
NeopixelVE neopixel;
LoggingPrinter LOGGER;

CO2Sensor co2Sensor;
SI7021Sensor si7021Sensor;
#ifndef HARDCODED_SENSORS
CustomHTTPDest customHTTPDest;
SerialDumpDest serialDumpDest;
TestSensor testSensor;
BME280Sensor bme280Sensor;
BMP085Sensor bmp085Sensor;
PM2005Sensor pm2005Sensor;
CDM7160Sensor cdm7160Sensor;
DS18B20Sensor _DS18B20Sensor;
LDR _LDR;

MQTTDest mqttDest;
RFDest rfDest;
DweetIOClass DweetIO;
WebServerClass WebServer;
ButtonClass Button;
TSL2561Sensor tsl2561Sensor;
BH1750Sensor bh1750Sensor;
GPIOClass gpioHandler;
APDS9960Sensor _APDS9960Sensor;
ServoV _ServoV;
AnnemometerClass annemometer;
BlynkDest blynkDest;
#endif
DestinationManagerClass DestinationManager;
