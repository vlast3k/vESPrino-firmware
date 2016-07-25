#include "commands/CommonCommands.hpp"
#include "MenuHandler.hpp"
#include <Arduino.h>
#include <Streaming.h>
//#include <EEPROM.h>
#include "common.hpp"


void CommonCommands::cmdHeap(const char *s) {
    Serial << "Heap: " << ESP.getFreeHeap() << endl;
}

void CommonCommands::dumpCfg(const char *s) {
  for (int i=0; i < 30; i++) {
    SERIAL << i << " : " ;
    for (int j=0; j < 100; j++) {
      byte b = EEPROM.read(i*100 + j);
      if (b == 0) b = '?';
      SERIAL << (char)b;
    }
    delay(10);
    SERIAL << endl;
  }
}

void CommonCommands::factoryReset(const char *ignore) {
  SERIAL << F("Doing Factory Reset, and restarting...") << endl;
  for (int i=0; i < 3000; i++) EEPROM.write(i, 0xFF);
  EEPROM.commit();
  ESP.restart();
}

void espRestart(const char* ignore) {
  ESP.restart();
}

void stopActiveExecution(const char *ignore) {
  SERIAL << "STOP Active execution\n";
  SKIP_LOOP = true;
}

void CommonCommands::registerCommands(MenuHandler *handler) {
  //MenuEntry *new MenuEntry(F("heap"), CMD_EXACT, &CommonCommands::cmdHeap, F("Free heap"));
  handler->registerCommand(new MenuEntry(F("factory"), CMD_EXACT, factoryReset, F("Return to defaults")));
  handler->registerCommand(new MenuEntry(F("debug"), CMD_EXACT, &CommonCommands::cmdDebug, F("Turn on debug")));
  handler->registerCommand(new MenuEntry(F("scani2c"), CMD_EXACT, scani2c, F("Scan I2C Bus")));
  handler->registerCommand(new MenuEntry(F("dumpCfg"), CMD_EXACT, &CommonCommands::dumpCfg, F("Dump EEPROM Contents")));
  handler->registerCommand(new MenuEntry(F("heap"), CMD_EXACT, &CommonCommands::cmdHeap, F("Free heap")));
  handler->registerCommand(new MenuEntry(F("restart"), CMD_EXACT, espRestart, F("")));
  handler->registerCommand(new MenuEntry(F("jscfg"), CMD_EXACT, printJSONConfig, F("")));
  handler->registerCommand(new MenuEntry(F("jjj"), CMD_EXACT, testJSON, F("")));
  handler->registerCommand(new MenuEntry(F("sss"), CMD_EXACT, stopActiveExecution, F("")));

}
