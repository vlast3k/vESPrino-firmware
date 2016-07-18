//H801 build with 1mb / 256k
//#define VTHING_H801_LED
#define VTHING_STARTER
//#define VAIR
//#define VTHING_CO2

#include "common.hpp"
#include <CubicGasSensors.h>



using namespace std;

  Timer *tmrStopLED;


NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod>  *strip;// = NeoPixelBus(1, D4);

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
void printVersion() {
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

  //Serial.begin(9600);
  //Serial1.begin(9600);
  //pinMode(0, INPUT);
  //pinMode(2, INPUT);
  //pgpio0 = digitalRead(0);
  //pgpio2 = digitalRead(2);
  printVersion();
  EEPROM.begin(3000);
  SERIAL << F("ready") << endl;

  SERIAL << F("Waiting for auto-connect") << endl;
  activeWait();

  #ifdef VTHING_STARTER
    initVThingStarter();
  #elif defined(VTHING_CO2)
    initCO2Handler();
  #elif defined(VTHING_H801_LED)
    h801_setup();
  #endif
}

void loop() {
  handleWifi();
  while (processUserInput()) delay(1000);
  if (SKIP_LOOP) {delay(100); return;}

  #ifdef VTHING_CO2
    loopCO2Handler();
  #elif defined(VTHING_STARTER)
    loopVThingStarter();
  #elif defined(VTHING_H801_LED)
    h801_loop();
  #endif
}
