//#define panic() __panic_func("", __LINE__, __func__)

//H801 build with 1mb / 256k
//#define VTHING_H801_LED
//#define VTHING_VESPRINO
//#define VAIR
//#define VTHING_CO2
//#include "ESP8266WiFi.h"
//#include "Arduino.h"
#include "common.hpp"
//#include <CubicGasSensors.h>
#include "commands/CommonCommands.hpp"
//#include "plugins/SAP_HCP_IOT_Plugin.hpp"
#include "plugins/AT_FW_Plugin.hpp"
#include "plugins/CustomURL_Plugin.hpp"
#include "plugins/URLShortcuts.hpp"
#include "plugins/PropertyList.hpp"
#include <LinkedList.h>
#include "interfaces/Sensor.hpp"
#include "interfaces/Plugin.hpp"
#include <Wire.h>
#include "plugins/NeopixelVE.hpp"
#include <I2CHelper.hpp>
#include "plugins/WifiStuff.hpp"
extern WifiStuffClass WifiStuff;
#include "plugins/TimerManager.hpp"
#include "plugins/DestinationManager.hpp"

extern TimerManagerClass TimerManager;
extern NeopixelVE neopixel; // there was a reason to put it here and not in commons

//#include <wiring_private.h>
using namespace std;

extern TimerManagerClass TimerManager;
extern DestinationManagerClass DestinationManager;
//  Timer *tmrStopLED;

//NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod>  *strip;// = NeoPixelBus(1, D4);

#ifdef VTHING_H801_LED
  #define SERIAL_PORT Serial1
#else
  #include <RunningAverage.h>
  #include <NeoPixelBus.h>
  #include <vSAP_Auth.h>

  //#define SERIAL_PORT LOGGER



#define PROP_SENSORS F("has.sensors")
#define PROP_STORED_I2C F("stored.i2c")
#endif
int pgpio0, pgpio2;

String VERSION = "v1.16";
void printVersion(const char* ignore) {
  LOGGER << endl;
  #ifdef VTHING_CO2
    LOGGER << F("vThing - CO2 Monitor ");
  #elif defined(VTHING_STARTER)
    LOGGER << F("vThing - Starter Edition ");
  #elif defined(VTHING_H801_LED)
    LOGGER << F("vThing - H801 FW ");
  #elif defined(VAIR)
    LOGGER << F("vAir - WiFi Module ");
  #elif defined(VTHING_VESPRINO)
    LOGGER << F("vESPrino ");
  #endif
  LOGGER << VERSION;
  LOGGER.flush();
  LOGGER << F(" build") << BUILD_NUM << endl;
  LOGGER.flush();
  //delay(100);
  LOGGER << F("IP address: ") << WiFi.localIP() << endl;
  LOGGER << F("Chip ID: ")    << _HEX(ESP.getChipId()) << endl;
  LOGGER.flush();
  //delay(100);

}

void registerPlugin(Plugin *plugin) {
  plugins.add(plugin);
}

void registerSensor(Sensor *sensor) {
  sensors.add(sensor);
}

void registerDestination(Destination *destination) {
  destinations.add(destination);
}

void setupPlugins(MenuHandler *handler) {
  if (DEBUG) LOGGER << F("\n--- Setup PLUGINS ---\n");
  for (int i=0; i < plugins.size(); i++) {
    if (DEBUG) LOGGER << plugins.get(i)->getName() << endl;
    if (plugins.get(i)->setup(handler)) LOGGER << F("Found: ") << plugins.get(i)->getName();
    WifiStuff.handleWifi();
    menuHandler.loop();
    delay(1);
  }

  if (DEBUG) LOGGER << F("\n--- Setup SENSORS ---\n");
  #ifdef HARDCODED_SENSORS
  storedSensors = HARDCODED_SENSORS;
  #else
  storedSensors = PropertyList.readProperty(PROP_SENSORS);
  #endif
  String foundSensors = "";
  for (int i=0; i < sensors.size(); i++) {
    String name = sensors.get(i)->getName();
    if (DEBUG) LOGGER << name << " : " << (storedSensors.indexOf(name) > -1) << endl;
    if (!storedSensors.length() || storedSensors.indexOf(name) > -1) {
      if (sensors.get(i)->setup(handler)) {
        LOGGER << F("Found: ") << name << endl;
        foundSensors += name + ",";
      }
    }
    WifiStuff.handleWifi();
    menuHandler.loop();
    delay(1);
  }
  if (storedSensors.length() && storedSensors != foundSensors) {
    LOGGER << F("Expected Sensors: ") << storedSensors << F(" found: ") << foundSensors << endl;
    LOGGER.flush();
    LOGGER << F("if the device configuration is changed, trigger a Factory Reset to update sensors\n");
    LOGGER.flush();
    #ifdef HARDCODED_SENSORS
    neopixel.cmdLedHandleColorInst(F("ledcolor seq95r"));
    #endif
  } else if (storedSensors.length() == 0) {
    if (!foundSensors.length()) foundSensors = "none";
    PropertyList.putProperty(PROP_SENSORS, foundSensors.c_str());
  } else {
    #ifdef HARDCODED_SENSORS
    neopixel.cmdLedHandleColorInst(F("ledcolor seq95g"));
    #endif
  }

  if (DEBUG) LOGGER << F("\n--- Setup DESTINATIONS ---\n");
  for (int i=0; i < destinations.size(); i++) {
    if (DEBUG) LOGGER << destinations.get(i)->getName() << endl;
    if (destinations.get(i)->setup(handler)) LOGGER << F("Found: ") << destinations.get(i)->getName();
    LOGGER.flush();
    WifiStuff.handleWifi();
    menuHandler.loop();
    delay(1);
  }
}

void loopPlugins() {
  //LOGGER << F("\n--- Loop PLUGINS ---\n");
  for (int i=0; i < plugins.size(); i++) {
  //  LOGGER << F("Loop plugin: ") << plugins.get(i)->getName() << endl;
    plugins.get(i)->loop();
    yield();
  }
  //S/erial << F("\n--- Loop SENSORS ---\n");
  for (int i=0; i < sensors.size(); i++) {
  //  LOGGER << F("Loop sensor: ") << sensors.get(i)->getName() << endl;
    sensors.get(i)->loop();
    yield();
  }
  //S/erial << F("\n--- Loop DESTINATIONS ---\n");
  for (int i=0; i < destinations.size(); i++) {
    //LOGGER << F("Loop Destination: ") << destinations.get(i)->getName() << endl;
    destinations.get(i)->loop();
    yield();
  }
  //LOGGER << F("\n--- Loop all DONE ---\n");
}

void reportProperty(String &key, String &value) {
  for (int i=0; i < plugins.size(); i++) {
    plugins.get(i)->onProperty(key, value);
    yield();
  }
  for (int i=0; i < sensors.size(); i++) {
    sensors.get(i)->onProperty(key, value);
    yield();
  }
  for (int i=0; i < destinations.size(); i++) {
    destinations.get(i)->onProperty(key, value);
    yield();
  }
}

// bool isDeepSleepWake() {
//   uint32_t dd;
//   ESP.rtcUserMemoryRead(0, &dd, sizeof(dd));
// //  LOGGER << "rtcUserMem: " << _HEX(dd) << endl;
//   if (dd == 33) {
//     return true;
//   } else {
//     dd = 33;
//     ESP.rtcUserMemoryWrite(0, &dd, sizeof(dd));
//     return false;
//   }
// }

// void onButton1() {
//   LOGGER << digitalRead(D3) << endl;
//   //if (digitalRead(D3) == 0) {
//     shouldSend = true;
//   //}
// }
//
// void attachButton1() {
//   //pinMode(BTTN_PIN, INPUT_PULLUP);
//   attachInterrupt(D3, onButton1, CHANGE);
// }

void fireEvent(const char *name) {
  String s = "event.";
  s += name;
  menuHandler.scheduleCommandProperty(s.c_str());
}

void setup() {
  Serial.begin(9600);
  if (DEBUG) heap("Heap at start");
  PERF("Setup a")
  PropertyList.begin(&menuHandler);
  PERF("Setup b")
  //heap("1");
  LOGGER.init();
  PERF("Setup c")
  rtcMemStore.init();
  PERF("Setup d")
  PropertyList.reportProperties();
  PERF("Setup e")
  PowerManager.setupInt(&menuHandler);
  PERF("Setup f")
  DestinationManager.onIterationStart();
  //heap("8");
  //heap("9");
  //heap("10");
  PERF("Setup 1")


  //DEBUG = PropertyList.readBoolProperty(PROP_DEBUG);
  //DEBUG = true;
  if (DEBUG) LOGGER << F("DEBUG is: ") << DEBUG;
  //heap("11");
  PERF("Setup 2")

  //Serial << "is woke from ds: " << PowerManager.isWokeFromDeepSleep() << endl;
  if (PowerManager.isWokeFromDeepSleep() == false) {
    neopixel.cmdLedSetBrgInst(F("ledbrg 99"));
    neopixel.cmdLedHandleColorInst(F("ledcolor lila"));
    neopixel.signal(LED_START);
    //activeWait();
  } else {
    neopixel.cmdLedSetBrgInst(F("ledbrg 99"));
    neopixel.cmdLedHandleColorInst(F("ledcolor black"));
    neopixel.signal(LED_START_DS);
  }
  PERF("Setup 3")

  //heap("2");
  if (DestinationManager.getWillSendThisIteration()) {
    WifiStuff.wifiConnectMulti();
  }
  yield();
  PERF("Setup 4")

  //heap("3");
  pinMode(D8, OUTPUT);    //enable power via D8
  digitalWrite(D8, HIGH);
  yield();

  // //delay(1000);
  //neopixel.cmdLedHandleColorInst(F("ledcolor green"));
  // pinMode(D8, OUTPUT);
  // digitalWrite(D8, HIGH);
  //Wire.begin(D6, D5);
  //LOGGER.flush();
  //delay(100);

  LOGGER.flush();
  //heap("4");
  //Wire.begin(D1, D6);
  //WiFi.begin();
  // #if defined(VTHING_CO2) || defined(VTHING_VESPRINO)
  //   LOGGER << "AAAA";
  //   // strip = new NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> (1, D4);
  //   // strip->Begin();
  //   // strip->SetPixelColor(0, RgbColor(5, 0,3));
  //   // strip->Show();
  //   setLedColor(RgbColor(5, 0,3));
  // #endif
  //WiFi.mode(WIFI_OFF);

  printVersion();
  yield();
  PERF("Setup 5")

  //heap("5");
  menuHandler.registerCommand(new MenuEntry(F("info"), CMD_EXACT, printVersion, F("")));
  //heap("6");

  CommonCommands commCmd;
  commCmd.registerCommands(&menuHandler);
  OTA_registerCommands(&menuHandler);
  //WIFI_registerCommands(&menuHandler);
  yield();
  PERF("Setup 6")

  //heap("7");

  //LOGGER << F("ready >") << endl;
  //LOGGER << F("Waiting for auto-connect") << endl;

//  deepSleepWake = isDeepSleepWake();
  uint16_t storedI2c = strtoul(PropertyList.readProperty(PROP_STORED_I2C), NULL, 0);
  I2C_STATE i2c= I2CHelper::beginI2C(PropertyList.readLongProperty(PROP_I2C_DISABLED_PORTS), storedI2c, &LOGGER);
  yield();
  switch (i2c) {
    case I2C_LOST: neopixel.signal(LED_LOST_I2C); break;
    case I2C_NODEVICES: neopixel.signal(LED_NO_I2C);
    case I2C_BEGIN: {
      uint16_t data = 0;
      data = data | (uint8_t)I2CHelper::i2cSDA;
      data = data | ((uint8_t)I2CHelper::i2cSCL << 8);
      if (storedI2c != data) {
        char val [10];
        sprintf(val, "0x%02X%02X", I2CHelper::i2cSCL, I2CHelper::i2cSDA);
        Serial << "i2c bus storing: " << val << endl;
        Serial.flush();
        PropertyList.putProperty(PROP_STORED_I2C, val);
      }

    }
  }
    //espRestart("");
  PERF("Setup 7")

  pinMode(D8, INPUT);

  //MigrateSettingsIfNeeded();
  //heap("12");

  EEPROM.begin(100);
  yield();

  //heap("13");
  PERF("Setup 8")

  // #ifdef VTHING_STARTER
  //   //initVThingStarter();
  // #elif defined(VTHING_CO2)
  //   //initCO2Handler();
  // #elif defined(VTHING_H801_LED)
  // //  h801_setup();
  // #endif

  //initCO2Handler();
  //registerDestination(&customHTTPDest);

  registerPlugin(&TimerManager);
  //heap("14");

  //registerPlugin(&PowerManager);
  setupPlugins(&menuHandler);
  PERF("Setup 9")

  #ifdef HARDCODED_SENSORS
  //delay(100000000L);
  #endif
  //heap("15");

  //SAP_HCP_IOT_Plugin::registerCommands(&menuHandler);
  AT_FW_Plugin::registerCommands(&menuHandler);
  //heap("bb");
  CustomURL_Plugin::registerCommands(&menuHandler);
  //heap("cc");

  URLShortcuts::registerCommands(&menuHandler);
  //heap("dd");
  PERF("Setup 10")

  //yield();

  //setup_IntThrHandler(&menuHandler);
  //PERF("Setup 11")

  initDecimalSeparator();
  PERF("Setup 12")

  if (DEBUG) heap("At setup end");
  LOGGER << F("Device initialized.") << endl;
  LOGGER.flush();
  yield();
  PERF("Setup 13")

  //if (DEBUG) fireEvent("setupEnd");
}

//int aa = 0;
uint32_t wfStart = 0;
bool checkedFUPD = false;
void loop() {
  WifiStuff.handleWifi();
  yield();
  menuHandler.loop();
  yield();
  if (SKIP_LOOP) {delay(100); return;}

  loopPlugins();
  //loop_IntThrHandler();
  menuHandler.loop();
  PowerManager.loopPowerManager();
  if (!PowerManager.isWokeFromDeepSleep() && !checkedFUPD) {
    checkedFUPD = true;
    menuHandler.scheduleCommand(F("fupd"));
  }
  //delay(1000);

}
