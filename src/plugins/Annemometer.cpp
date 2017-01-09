#include "plugins/Annemometer.hpp"
#include "MenuHandler.hpp"
#include "plugins/GPIO.hpp"
#include <Timer.h>

extern AnnemometerClass annemometer;

#define PROP_ANNEM_CONNECTED F("annem.connected")

AnnemometerClass::AnnemometerClass() {
  registerPlugin(this);
}

bool AnnemometerClass::setup(MenuHandler *handler) {
   handler->registerCommand(new MenuEntry(F("annemStart"), CMD_BEGIN, AnnemometerClass::cmdStart, F("annemStart D7 - port number start annemometer")));
   String s = PropertyList.readProperty(PROP_ANNEM_CONNECTED);
   if (s.length()) {
     s = String("annemStart ") + s;
     menuHandler.scheduleCommand(s.c_str());
   }
   return false;
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
  uint32_t disabledPorts = PropertyList.readLongProperty(PROP_I2C_DISABLED_PORTS);
  if (!GPIOClass::isBitSet(disabledPorts, port)) {
    GPIOClass::setBit(disabledPorts, port, 1);
    String dp = String(disabledPorts);
    PropertyList.putProperty(PROP_I2C_DISABLED_PORTS, dp.c_str());
  }
  menuHandler.scheduleCommand("nop 0");
}

void AnnemometerClass::loop() {
  if (!started) return;
  if (millis() - lastSent < 15000) return;

  //LOGGER << "StartMasure!" << endl;
  int state = 0;
  uint32_t loopStart = millis();
  int hasWind = 0;
  int threshold = 25;
  uint32_t lastHigh = 0;
  state = digitalRead(port);
  while (millis() - loopStart < 1000) {
    if (digitalRead(port) != state) {
      state = !state;
      if (state == 1) {
        int spd = millis() - lastHigh;
        //LOGGER << spd << endl;
        lastHigh = millis();
        if (spd > 5 && spd < threshold) hasWind ++;
        else { hasWind = 0;}
        if (hasWind > 30) {
          fireEvent("onStrongWind");
          lastSent = millis();
          return;
        }
      }
    }
  }
}
