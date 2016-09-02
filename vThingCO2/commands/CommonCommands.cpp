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

void cmdTestMemStore(const char *ignore) {
  SERIAL << "Test Mem Store\n";
  Serial << "avg now 1: "<<     rtcMemStore.getAverage() << endl;
  rtcMemStore.addAverageValue(1000);
  Serial << "avg now 2:"<<     rtcMemStore.getAverage() << endl;
  rtcMemStore.addAverageValue(2000);
  Serial << "avg now 3:"<<     rtcMemStore.getAverage() << endl;
  rtcMemStore.addAverageValue(2000);
  Serial << "avg now 4:"<<     rtcMemStore.getAverage() << endl;
  rtcMemStore.addAverageValue(2000);
  Serial << "avg now 5:"<<     rtcMemStore.getAverage() << endl;
  SKIP_LOOP = true;
}
void cmdSerTest(const char *p) {
  char p1[20],  p2[20];
  p = extractStringFromQuotes(p, p1, 20);
  //p = extractStringFromQuotes(p, p2, 20);
  String s;
  int strLen = atoi(p1);
  for (int i=0; i < strLen; i++) s+=".";
  //s += "\n";
  for (int i=0; i<30; i++) {
    Serial << s <<endl;
    delay(1);
  }
  Serial.flush();
  //Serial << endl << "EEENDDD" << endl;
  //delay(5000);
  // int iport = atoi(port);
  // WiFiClient ccc;
  // SERIAL << "Test connection to to:" << host << ":" << port << endl;
  // int res = ccc.connect(host, iport);
  // SERIAL << "Res: " << res << endl;

}

void CommonCommands::registerCommands(MenuHandler *handler) {
  //MenuEntry *new MenuEntry(F("heap"), CMD_EXACT, &CommonCommands::cmdHeap, F("Free heap"));
  handler->registerCommand(new MenuEntry(F("factory"), CMD_EXACT, factoryReset, F("Return to defaults")));
  handler->registerCommand(new MenuEntry(F("debug"), CMD_EXACT, &CommonCommands::cmdDebug, F("Turn on debug")));
  handler->registerCommand(new MenuEntry(F("scani2c"), CMD_EXACT, cmdScanI2C, F("Scan I2C Bus")));
  handler->registerCommand(new MenuEntry(F("dumpi2c"), CMD_EXACT, dumpI2CBus, F("Scan I2C Bus")));
  handler->registerCommand(new MenuEntry(F("dumpCfg"), CMD_EXACT, &CommonCommands::dumpCfg, F("Dump EEPROM Contents")));
  handler->registerCommand(new MenuEntry(F("heap"), CMD_EXACT, &CommonCommands::cmdHeap, F("Free heap")));
  handler->registerCommand(new MenuEntry(F("restart"), CMD_EXACT, espRestart, F("")));
  // handler->registerCommand(new MenuEntry(F("jscfg"), CMD_EXACT, printJSONConfig, F("")));
  // handler->registerCommand(new MenuEntry(F("jjj"), CMD_EXACT, testJSON, F("")));
  handler->registerCommand(new MenuEntry(F("sss"), CMD_EXACT, stopActiveExecution, F("")));
  handler->registerCommand(new MenuEntry(F("tms"), CMD_EXACT, cmdTestMemStore, F("")));
  handler->registerCommand(new MenuEntry(F("sertest"), CMD_BEGIN, cmdSerTest, F("")));

}
