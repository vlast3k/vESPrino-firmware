//H801 build with 1mb / 256k
//#define VTHING_H801_LED
//#define VTHING_VESPRINO
//#define VAIR
//#define VTHING_CO2

#include "common.hpp"
#include <CubicGasSensors.h>
#include "commands\CommonCommands.hpp"
#include "plugins\SAP_HCP_IOT_Plugin.hpp"
#include "plugins\AT_FW_Plugin.hpp"
#include "plugins\CustomURL_Plugin.hpp"
#include "plugins\URLShortcuts.hpp"
#include "plugins\PropertyList.hpp"
#include <LinkedList.h>
#include "interfaces\Sensor.hpp"
#include "interfaces\Plugin.hpp"
#include "destinations\CustomHTTPDest.hpp"
#include <Wire.h>



using namespace std;

//  Timer *tmrStopLED;


//NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod>  *strip;// = NeoPixelBus(1, D4);

#ifdef VTHING_H801_LED
  #define SERIAL Serial1
#else
  #include <RunningAverage.h>
  #include <NeoPixelBus.h>
  #include <vSAP_Auth.h>

  #define SERIAL Serial





#endif
int pgpio0, pgpio2;

String VERSION = "v1.16";
void printVersion(const char* ignore) {
  SERIAL << endl;
  #ifdef VTHING_CO2
    SERIAL << F("vThing - CO2 Monitor ");
  #elif defined(VTHING_STARTER)
    SERIAL << F("vThing - Starter Edition ");
  #elif defined(VTHING_H801_LED)
    SERIAL << F("vThing - H801 FW ");
  #elif defined(VAIR)
    SERIAL << F("vAir - WiFi Module ");
  #elif defined(VTHING_VESPRINO)
    SERIAL << F("vESPrino ");
  #endif
  SERIAL << VERSION << endl;
  SERIAL << F("IP address: ") << WiFi.localIP() << endl;

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
  Serial << "Plugin.setup()\n";
  for (int i=0; i < plugins.size(); i++) {
  //  Serial << "Setup plugins:" << plugins.get(i)->getName() << endl;
    plugins.get(i)->setup(handler);
  }
  Serial << "Sensor.setup()\n";
  for (int i=0; i < sensors.size(); i++) {
//    Serial << "Setup sensors:" << sensors.get(i)->getName() << endl;
    sensors.get(i)->setup(handler);
  }
  Serial << "Destination.setup()\n";
  for (int i=0; i < destinations.size(); i++) {
//    Serial << "Setup Destinations:" << destinations.get(i)->getName() << endl;
    destinations.get(i)->setup(handler);
  }
}

void loopPlugins() {
  for (int i=0; i < plugins.size(); i++) {
    //Serial << "Loop plugins:" << plugins.get(i)->getName() << endl;
    plugins.get(i)->loop();
  }
  for (int i=0; i < sensors.size(); i++) {
//    Serial << "Loop sensors:" << sensors.get(i)->getName() << endl;
    sensors.get(i)->loop();
  }
  for (int i=0; i < destinations.size(); i++) {
//    Serial << "Loop Destinations:" << destinations.get(i)->getName() << endl;
    destinations.get(i)->loop();
  }
}

bool isDeepSleepWake() {
  uint32_t dd;
  ESP.rtcUserMemoryRead(0, &dd, sizeof(dd));
  Serial << "rtcUserMem: " << _HEX(dd) << endl;
  if (dd == 33) {
    return true;
  } else {
    dd = 33;
    ESP.rtcUserMemoryWrite(0, &dd, sizeof(dd));
    return false;
  }
}

void setup() {
  SERIAL.begin(9600);
  //Wire.begin(D6, D5);
  beginI2C();
  //Wire.begin(D1, D6);
  //WiFi.begin();
  #if defined(VTHING_CO2) || defined(VTHING_STARTER)
    SERIAL << "AAAA";
    strip = new NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> (1, D4);
    strip->Begin();
    strip->SetPixelColor(0, RgbColor(20, 0, 10));
    strip->Show();
  #endif

  pinMode(D8, OUTPUT);    //enable power via D8
  digitalWrite(D8, HIGH);
  delay(1000);

  printVersion();
  SERIAL << F("ready") << endl;
  SERIAL << F("Waiting for auto-connect") << endl;

  deepSleepWake = isDeepSleepWake();

  if (!deepSleepWake) activeWait();
  PropertyList.begin();
  MigrateSettingsIfNeeded();



  #ifdef VTHING_STARTER
    //initVThingStarter();
  #elif defined(VTHING_CO2)
    //initCO2Handler();
  #elif defined(VTHING_H801_LED)
    h801_setup();
  #endif

  //initCO2Handler();
  //registerDestination(&customHTTPDest);

  registerPlugin(&TimerManager);
  registerPlugin(&PowerManager);
  setupPlugins(&menuHandler);

  CommonCommands commCmd;
  commCmd.registerCommands(&menuHandler);
  SAP_HCP_IOT_Plugin::registerCommands(&menuHandler);
  AT_FW_Plugin::registerCommands(&menuHandler);
  CustomURL_Plugin::registerCommands(&menuHandler);
  URLShortcuts::registerCommands(&menuHandler);
  OTA_registerCommands(&menuHandler);
  WIFI_registerCommands(&menuHandler);
  MQTT_RegisterCommands(&menuHandler);
#ifdef VTHING_CO2
  //CO2_registerCommands(&menuHandler);
#endif
#ifdef VTHING_H801_LED
  H801_registerCommands(&menuHandler);
#endif
#ifdef VTHING_VESPRINO
  VESP_registerCommands(&menuHandler);
#endif
menuHandler.registerCommand(new MenuEntry(F("info"), CMD_EXACT, printVersion, F("")));

  setup_IntThrHandler(&menuHandler);
  heap("");
  //WiFi.begin("MarinaResidence","eeeeee");
}
//int aa = 0;
void loop() {
  //if ((aa++ % 500) == 0) Serial << "." << endl;
  handleWifi();
  menuHandler.loop();
  if (SKIP_LOOP) {delay(100); return;}
  loopPlugins();
  #ifdef VTHING_CO2
  //  loopCO2Handler();
  #elif defined(VTHING_VESPRINO)
  //  loopVThingStarter();
  #elif defined(VTHING_H801_LED)
    h801_loop();
  #endif

  loop_IntThrHandler();
  menuHandler.loop();
  PowerManager.loopPowerManager();
  //delay(1000);
}
