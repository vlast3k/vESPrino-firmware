# 1 "c:\\users\\vladi\\appdata\\local\\temp\\tmpcphyeh"
#include <Arduino.h>
# 1 "C:/develop/arduino/vThingCO2/src/vThingCO2.ino"
# 15 "C:/develop/arduino/vThingCO2/src/vThingCO2.ino"
#include "common.hpp"



#include "commands/CommonCommands.hpp"



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





using namespace std;
# 61 "C:/develop/arduino/vThingCO2/src/vThingCO2.ino"
#ifdef VTHING_H801_LED

  #define SERIAL_PORT Serial1

#else

  #include <RunningAverage.h>

  #include <NeoPixelBus.h>

  #include <vSAP_Auth.h>



  #define SERIAL_PORT Serial
# 87 "C:/develop/arduino/vThingCO2/src/vThingCO2.ino"
#endif

int pgpio0, pgpio2;



String VERSION = "v1.16";
void printVersion(const char* ignore);
void registerPlugin(Plugin *plugin);
void registerSensor(Sensor *sensor);
void registerDestination(Destination *destination);
void setupPlugins(MenuHandler *handler);
void loopPlugins();
void fireEvent(const char *name);
void setup();
void loop();
#line 95 "C:/develop/arduino/vThingCO2/src/vThingCO2.ino"
void printVersion(const char* ignore) {

  SERIAL_PORT << endl;

  #ifdef VTHING_CO2

    SERIAL_PORT << F("vThing - CO2 Monitor ");

  #elif defined(VTHING_STARTER)

    SERIAL_PORT << F("vThing - Starter Edition ");

  #elif defined(VTHING_H801_LED)

    SERIAL_PORT << F("vThing - H801 FW ");

  #elif defined(VAIR)

    SERIAL_PORT << F("vAir - WiFi Module ");

  #elif defined(VTHING_VESPRINO)

    SERIAL_PORT << F("vESPrino ");

  #endif

  SERIAL_PORT << VERSION << F(" build") << BUILD_NUM << endl;

  Serial.flush();

  delay(100);

  SERIAL_PORT << F("IP address: ") << WiFi.localIP() << endl;

  SERIAL_PORT << F("Chip ID: ") << _HEX(ESP.getChipId()) << endl;

  Serial.flush();

  delay(100);



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



  Serial << F("\n--- Setup PLUGINS ---\n");

  for (int i=0; i < plugins.size(); i++) {

    Serial << F("Setup plugin: ") << plugins.get(i)->getName() << endl;

    plugins.get(i)->setup(handler);

  }

  Serial << F("\n--- Setup SENSORS ---\n");

  for (int i=0; i < sensors.size(); i++) {

    Serial << F("Setup sensor: ") << sensors.get(i)->getName() << endl;

    sensors.get(i)->setup(handler);

  }

  Serial << F("\n--- Setup DESTINATIONS ---\n");

  for (int i=0; i < destinations.size(); i++) {

    Serial << F("Setup Destination: ") << destinations.get(i)->getName() << endl;

    destinations.get(i)->setup(handler);

  }





}



void loopPlugins() {



  for (int i=0; i < plugins.size(); i++) {



    plugins.get(i)->loop();

  }



  for (int i=0; i < sensors.size(); i++) {



    sensors.get(i)->loop();

  }



  for (int i=0; i < destinations.size(); i++) {



    destinations.get(i)->loop();

  }



}
# 295 "C:/develop/arduino/vThingCO2/src/vThingCO2.ino"
void fireEvent(const char *name) {

  String s = "event.";

  s += name;

  menuHandler.scheduleCommandProperty(s.c_str());

}



extern NeopixelVE neopixel;

void setup() {
# 323 "C:/develop/arduino/vThingCO2/src/vThingCO2.ino"
  SERIAL_PORT.begin(9600);

  PropertyList.begin(&menuHandler);



  Serial.flush();

  delay(100);



  heap("Heap at start");

  Serial.flush();

  I2CHelper::beginI2C();
# 365 "C:/develop/arduino/vThingCO2/src/vThingCO2.ino"
  printVersion();

  menuHandler.registerCommand(new MenuEntry(F("info"), CMD_EXACT, printVersion, F("")));

  CommonCommands commCmd;

  commCmd.registerCommands(&menuHandler);

  OTA_registerCommands(&menuHandler);

  WIFI_registerCommands(&menuHandler);



  SERIAL_PORT << F("ready >") << endl;

  SERIAL_PORT << F("Waiting for auto-connect") << endl;





  rtcMemStore.init();

  PowerManager.setup(&menuHandler);







  DEBUG = PropertyList.readBoolProperty(PROP_DEBUG);

  if (DEBUG) Serial << F("DEBUG is: ") << DEBUG;





  if (PowerManager.isWokeFromDeepSleep() == false) {

    activeWait();

    menuHandler.scheduleCommand(F("fupd"));

    neopixel.cmdLedSetBrgInst(F("ledbrg 99"));

    neopixel.cmdLedHandleColorInst(F("ledcolor lila"));

  } else {

    neopixel.cmdLedSetBrgInst(F("ledbrg 99"));

    neopixel.cmdLedHandleColorInst(F("ledcolor black"));

  }



  wifiConnectMulti();



  MigrateSettingsIfNeeded();



  EEPROM.begin(100);
# 461 "C:/develop/arduino/vThingCO2/src/vThingCO2.ino"
  registerPlugin(&TimerManager);



  setupPlugins(&menuHandler);





  AT_FW_Plugin::registerCommands(&menuHandler);

  CustomURL_Plugin::registerCommands(&menuHandler);

  URLShortcuts::registerCommands(&menuHandler);

  MQTT_RegisterCommands(&menuHandler);

#ifdef VTHING_CO2



#endif

#ifdef VTHING_H801_LED

  H801_registerCommands(&menuHandler);

#endif

#ifdef VTHING_VESPRINO



#endif



  setup_IntThrHandler(&menuHandler);
# 511 "C:/develop/arduino/vThingCO2/src/vThingCO2.ino"
  heap("At setup end");

  fireEvent("setupEnd");
# 529 "C:/develop/arduino/vThingCO2/src/vThingCO2.ino"
}



uint32_t wfStart = 0;

void loop() {
# 583 "C:/develop/arduino/vThingCO2/src/vThingCO2.ino"
  handleWifi();

  menuHandler.loop();

  if (SKIP_LOOP) {delay(100); return;}



  loopPlugins();

  loop_IntThrHandler();

  menuHandler.loop();

  PowerManager.loopPowerManager();





}