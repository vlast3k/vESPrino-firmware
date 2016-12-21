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

//#include <wiring_private.h>
using namespace std;


//  Timer *tmrStopLED;


//NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod>  *strip;// = NeoPixelBus(1, D4);

#ifdef VTHING_H801_LED
  #define SERIAL_PORT Serial1
#else
  #include <RunningAverage.h>
  #include <NeoPixelBus.h>
  #include <vSAP_Auth.h>

  #define SERIAL_PORT Serial





#endif
int pgpio0, pgpio2;

String VERSION = "v1.16";
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
  SERIAL_PORT << VERSION;
  Serial.flush();
  Serial << F(" build") << BUILD_NUM << endl;
  Serial.flush();
  //delay(100);
  SERIAL_PORT << F("IP address: ") << WiFi.localIP() << endl;
  SERIAL_PORT << F("Chip ID: ")    << _HEX(ESP.getChipId()) << endl;
  Serial.flush();
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
  //menuHandler.handleCommand(F("scani2c"));
  Serial << F("\n--- Setup PLUGINS ---\n");
  for (int i=0; i < plugins.size(); i++) {
//    Serial << F("Setup plugin: ") << plugins.get(i)->getName() << endl;
    Serial << plugins.get(i)->getName() << endl;
    plugins.get(i)->setup(handler);
    menuHandler.loop();
    delay(1);

  }
  Serial << F("\n--- Setup SENSORS ---\n");
  for (int i=0; i < sensors.size(); i++) {
//    Serial << F("Setup sensor: ") << sensors.get(i)->getName() << endl;
    Serial << sensors.get(i)->getName() << endl;
    sensors.get(i)->setup(handler);
    menuHandler.loop();
    delay(1);

  }
  Serial << F("\n--- Setup DESTINATIONS ---\n");
  for (int i=0; i < destinations.size(); i++) {
    Serial << destinations.get(i)->getName() << endl;
    //Serial << F("Setup Destination: ") << destinations.get(i)->getName() << endl;
    destinations.get(i)->setup(handler);
    menuHandler.loop();
    delay(1);

  }
//  Serial << F("\n--- Setup DONE ---\n");

}

void loopPlugins() {
  //Serial << F("\n--- Loop PLUGINS ---\n");
  for (int i=0; i < plugins.size(); i++) {
  //  Serial << F("Loop plugin: ") << plugins.get(i)->getName() << endl;
    plugins.get(i)->loop();
    delay(1);
  }
  //S/erial << F("\n--- Loop SENSORS ---\n");
  for (int i=0; i < sensors.size(); i++) {
  //  Serial << F("Loop sensor: ") << sensors.get(i)->getName() << endl;
    sensors.get(i)->loop();
    delay(1);
  }
  //S/erial << F("\n--- Loop DESTINATIONS ---\n");
  for (int i=0; i < destinations.size(); i++) {
    //Serial << F("Loop Destination: ") << destinations.get(i)->getName() << endl;
    destinations.get(i)->loop();
    delay(1);
  }
  //Serial << F("\n--- Loop all DONE ---\n");
}

// bool isDeepSleepWake() {
//   uint32_t dd;
//   ESP.rtcUserMemoryRead(0, &dd, sizeof(dd));
// //  Serial << "rtcUserMem: " << _HEX(dd) << endl;
//   if (dd == 33) {
//     return true;
//   } else {
//     dd = 33;
//     ESP.rtcUserMemoryWrite(0, &dd, sizeof(dd));
//     return false;
//   }
// }

// void onButton1() {
//   Serial << digitalRead(D3) << endl;
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

extern NeopixelVE neopixel; // there was a reason to put it here and not in commons
void setup() {
  SERIAL_PORT.begin(9600);
  PropertyList.begin(&menuHandler);
  wifiConnectMulti();
  pinMode(D8, OUTPUT);    //enable power via D8
  digitalWrite(D8, HIGH);
  // //delay(1000);
  //neopixel.cmdLedHandleColorInst(F("ledcolor green"));
  // pinMode(D8, OUTPUT);
  // digitalWrite(D8, HIGH);
  //Wire.begin(D6, D5);
  //Serial.flush();
  //delay(100);

  heap("Heap at start");
  Serial.flush();
  //Wire.begin(D1, D6);
  //WiFi.begin();
  // #if defined(VTHING_CO2) || defined(VTHING_VESPRINO)
  //   SERIAL_PORT << "AAAA";
  //   // strip = new NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> (1, D4);
  //   // strip->Begin();
  //   // strip->SetPixelColor(0, RgbColor(5, 0,3));
  //   // strip->Show();
  //   setLedColor(RgbColor(5, 0,3));
  // #endif
  //WiFi.mode(WIFI_OFF);

  printVersion();
  menuHandler.registerCommand(new MenuEntry(F("info"), CMD_EXACT, printVersion, F("")));
  CommonCommands commCmd;
  commCmd.registerCommands(&menuHandler);
  OTA_registerCommands(&menuHandler);
  WIFI_registerCommands(&menuHandler);

  SERIAL_PORT << F("ready >") << endl;
  SERIAL_PORT << F("Waiting for auto-connect") << endl;

//  deepSleepWake = isDeepSleepWake();
  rtcMemStore.init();
  PowerManager.setup(&menuHandler);
  //I2CHelper::beginI2C(PropertyList.readLongProperty(PROP_I2C_DISABLED_PORTS));
  pinMode(D8, INPUT);


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

  //MigrateSettingsIfNeeded();

  EEPROM.begin(100);


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
  //registerPlugin(&PowerManager);
  setupPlugins(&menuHandler);

  //SAP_HCP_IOT_Plugin::registerCommands(&menuHandler);
  AT_FW_Plugin::registerCommands(&menuHandler);
  CustomURL_Plugin::registerCommands(&menuHandler);
  URLShortcuts::registerCommands(&menuHandler);
  //MQTT_RegisterCommands(&menuHandler);
#ifdef VTHING_CO2
  //CO2_registerCommands(&menuHandler);
#endif
#ifdef VTHING_H801_LED
  H801_registerCommands(&menuHandler);
#endif
#ifdef VTHING_VESPRINO
//  VESP_registerCommands(&menuHandler);
#endif

  setup_IntThrHandler(&menuHandler);

  //rfDest.sendPing(1000);

//  menuHandler.scheduleCommand(F("sendNow"));

  heap("At setup end");
  fireEvent("setupEnd");
  //neopixel.cmdLedHandleColorInst(F("ledcolor red"));
  //setLedColor(RgbColor(5, 0,3));
  //WiFi.begin("MarinaResidence","eeeeee");
  //menuHandler.scheduleCommand("nop 0");
  //menuHandler.scheduleCommand("oled Started");

}
//int aa = 0;
uint32_t wfStart = 0;
void loop() {
  //if ((aa++ % 500) == 0) Serial << "." << endl;
//   if (shouldSend == false && digitalRead(D3) == 0) {
//     Serial << millis() - wfStart << endl;
//     if (wfStart == 0) {
//       wfStart =millis();
//       neopixel.cmdLedSetBrgInst(F("ledbrg 90"));
//       neopixel.cmdLedHandleColorInst(F("ledcolor cyan"));
//     } else if (millis() - wfStart > 4000) {
//       //Serial << "SSSSSSSSSSSS" << endl;
//       startAutoWifiConfig();
//       shouldSend = true;
//     }
//     delay(100);
//     return;
//   } else if (wfStart > 0) {
//     wfStart = 0;
//     neopixel.cmdLedSetBrgInst(F("ledbrg 99"));
//     neopixel.cmdLedHandleColorInst(F("ledcolor black"));
// //    menuHandler.scheduleCommand("cdmloop");
//   }



  handleWifi();
  menuHandler.loop();
  if (SKIP_LOOP) {delay(100); return;}

  loopPlugins();
  loop_IntThrHandler();
  menuHandler.loop();
  PowerManager.loopPowerManager();
  //delay(1000);

}
