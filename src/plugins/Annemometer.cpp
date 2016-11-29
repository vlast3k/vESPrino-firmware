#include "plugins/Annemometer.hpp"
#include "MenuHandler.hpp"
#include "plugins/GPIO.hpp"
#include <Timer.h>

extern AnnemometerClass annemometer;

#define PROP_ANNEM_CONNECTED F("annem.connected")

AnnemometerClass::AnnemometerClass() {
  registerPlugin(this);
}

void AnnemometerClass::setup(MenuHandler *handler) {
   handler->registerCommand(new MenuEntry(F("annemStart"), CMD_BEGIN, AnnemometerClass::cmdStart, F("annemStart D7 - port number start annemometer")));
   String s = PropertyList.readProperty(PROP_ANNEM_CONNECTED);
   if (s.length()) {
     s = String("annemStart ") + s;
     cmdStartInst(s.c_str());
   }
}

void AnnemometerClass::cmdStart(const char *line) {
  annemometer.cmdStartInst(line);
}

void AnnemometerClass::cmdStartInst(const char *line) {
  char *s = strchr(line, ' ');
  if (!s) return;
  port = GPIOClass::convertToGPIO(s+1);
  started = true;
  pinMode(port, INPUT);
  menuHandler.scheduleCommand("nop 0");
}

void AnnemometerClass::loop() {
  if (!started) return;
  Serial << "StartMasure!" << endl;
  int state = 0;
  uint32_t loopStart = millis();
  int hasWind = 0;
  int threshold = 20;
  uint32_t lastHigh = 0;
  state = digitalRead(port);
  while (millis() - loopStart < 1000) {
    if (digitalRead(port) != state) {
      state = !state;
      if (state == 1) {
        int spd = millis() - lastHigh;
        Serial << spd << endl;
        lastHigh = millis();
        if (spd < threshold) hasWind ++;
        else { hasWind = 0;}
        if (hasWind > 30) {
          Serial << "Techenie!" << endl;
          hasWind = 0;
        }
      }
    }
  }
}
