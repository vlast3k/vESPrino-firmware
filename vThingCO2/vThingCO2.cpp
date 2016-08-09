//H801 build with 1mb / 256k
//#define VTHING_H801_LED
#define VTHING_STARTER
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



  void onStopLED() {
   // SERIAL << "STOP LED Executed " << endl;
      strip->SetPixelColor(0, RgbColor(0, 0,0));
      strip->Show();
  }

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

void registerSensor(Destination *destination) {
  destinations.add(destination);
}



void setup() {
  SERIAL.begin(9600);

  //WiFi.begin();
  #if defined(VTHING_CO2) || defined(VTHING_STARTER)
  SERIAL << "AAAA";
    strip = new NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> (1, D4);
    strip->Begin();
    strip->SetPixelColor(0, RgbColor(20, 0, 10));
    strip->Show();
  #endif

  pinMode(D8, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  digitalWrite(D8, HIGH);
  delay(1000);

  //Serial.begin(9600);
  //Serial1.begin(9600);
  //pinMode(0, INPUT);
  //pinMode(2, INPUT);
  //pgpio0 = digitalRead(0);
  //pgpio2 = digitalRead(2);
  printVersion();

  SERIAL << F("ready") << endl;

  SERIAL << F("Waiting for auto-connect") << endl;
  activeWait();
  MigrateSettingsIfNeeded();
  PropertyList.begin();

  #ifdef VTHING_STARTER
    initVThingStarter();
  #elif defined(VTHING_CO2)
    initCO2Handler();
  #elif defined(VTHING_H801_LED)
    h801_setup();
  #endif
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
  CO2_registerCommands(&menuHandler);
#endif
#ifdef VTHING_H801_LED
  H801_registerCommands(&menuHandler);
#endif
#ifdef VTHING_STARTER
  VESP_registerCommands(&menuHandler);
#endif
  menuHandler.registerCommand(new MenuEntry(F("info"), CMD_EXACT, printVersion, F("")));

 setup_IntThrHandler();
}

void loop() {
  handleWifi();
  while (menuHandler.processUserInput()) delay(1000);
  if (SKIP_LOOP) {delay(100); return;}

  #ifdef VTHING_CO2
    loopCO2Handler();
  #elif defined(VTHING_STARTER)
    loopVThingStarter();
  #elif defined(VTHING_H801_LED)
    h801_loop();
  #endif
  loop_IntThrHandler();
}
