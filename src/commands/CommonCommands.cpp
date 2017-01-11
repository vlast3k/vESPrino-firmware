#include "commands/CommonCommands.hpp"
#include "MenuHandler.hpp"
#include <Arduino.h>
#include <Streaming.h>
#include "sensors/CO2Sensor.hpp"
//#include <EEPROM.h>
#include "common.hpp"


void CommonCommands::cmdHeap(const char *s) {
    LOGGER << F("Heap: ") << ESP.getFreeHeap() << endl;
}

void CommonCommands::cmdDebug(const char *s) {
  if (strcmp(s, "debug") == 0) {
    DEBUG = !DEBUG;
    PropertyList.putProperty(PROP_DEBUG, DEBUG ? "1" : "0");
  } else {
    DEBUG = PropertyList.readBoolProperty(PROP_DEBUG);
  }
  LOGGER << F("Debug is now: ") << (DEBUG ? F("ENABLED") : F("DISABLED")) << endl;
}

void CommonCommands::dumpCfg(const char *s) {
  for (int i=0; i < 30; i++) {
    LOGGER << i << F(" : ");
    for (int j=0; j < 100; j++) {
      byte b = EEPROM.read(i*100 + j);
      if (b == 0) b = '?';
      LOGGER << (char)b;
    }
    delay(10);
    LOGGER << endl;
  }
}
void espRestart(const char* ignore) {
  menuHandler.handleCommand("wss_stop");
  rtcMemStore.clear();
  ESP.restart();
}

void CommonCommands::factoryReset(const char *ignore) {
  LOGGER << F("Doing Factory Reset, and restarting...") << endl;
  //LOGGER << "was in deep sleep: "<< rtcMemStore.wasInDeepSleep()<< endl;
  // for (int i=0; i < 100; i++) EEPROM.write(i, 0xFF);
  // EEPROM.commit();
  bool fullReset = (strcmp(ignore, "factoryr") == 0);
  String tempAdj, co2Ps;
  if (!fullReset) {
    tempAdj = PropertyList.readProperty(PROP_TEMP_ADJ);
    co2Ps = PropertyList.readProperty(PROP_CUBIC_CO2_POWERSAFE);
  }
  PropertyList.factoryReset();
  if (!fullReset) {
    PropertyList.putProperty(PROP_TEMP_ADJ, tempAdj.c_str());
    PropertyList.putProperty(PROP_CUBIC_CO2_POWERSAFE, co2Ps.c_str());
  }
  LOGGER.flush();
  delay(100);
  espRestart(NULL);
}



void stopActiveExecution(const char *ignore) {
  LOGGER << F("STOP Active execution\n");
  SKIP_LOOP = true;
}

// void cmdTestMemStore(const char *ignore) {
//   LOGGER << "Test Mem Store\n";
//   LOGGER << "avg now 1: "<<     rtcMemStore.getAverage() << endl;
//   rtcMemStore.addAverageValue(1000);
//   LOGGER << "avg now 2:"<<     rtcMemStore.getAverage() << endl;
//   rtcMemStore.addAverageValue(2000);
//   LOGGER << "avg now 3:"<<     rtcMemStore.getAverage() << endl;
//   rtcMemStore.addAverageValue(2000);
//   LOGGER << "avg now 4:"<<     rtcMemStore.getAverage() << endl;
//   rtcMemStore.addAverageValue(2000);
//   LOGGER << "avg now 5:"<<     rtcMemStore.getAverage() << endl;
//   SKIP_LOOP = true;
// }
void cmdSerTest(const char *p) {
  char p1[20],  p2[20];
  p = extractStringFromQuotes(p, p1, 20);
  //p = extractStringFromQuotes(p, p2, 20);
  String s;
  int strLen = atoi(p1);
  for (int i=0; i < strLen; i++) s+=".";
  //s += "\n";
  for (int i=0; i<30; i++) {
    LOGGER << s <<endl;
    delay(1);
  }
  LOGGER.flush();
  //LOGGER << endl << "EEENDDD" << endl;
  //delay(5000);
  // int iport = atoi(port);
  // WiFiClient ccc;
  // LOGGER << "Test connection to to:" << host << ":" << port << endl;
  // int res = ccc.connect(host, iport);
  // LOGGER << "Res: " << res << endl;

}

void CommonCommands::registerCommands(MenuHandler *handler) {
  //MenuEntry *new MenuEntry(F("heap"), CMD_EXACT, &CommonCommands::cmdHeap, F("Free heap"));
  handler->registerCommand(new MenuEntry(F("factory"), CMD_BEGIN, factoryReset, F("Return to defaults")));
  handler->registerCommand(new MenuEntry(F("debug"), CMD_BEGIN, &CommonCommands::cmdDebug, F("Toggle debug")));
  handler->registerCommand(new MenuEntry(F("scani2c"), CMD_EXACT, I2CHelper::cmdScanI2C, F("Scan I2C Bus")));
  handler->registerCommand(new MenuEntry(F("dumpi2c"), CMD_EXACT, I2CHelper::dumpI2CBus, F("Scan I2C Bus")));
  handler->registerCommand(new MenuEntry(F("dumpCfg"), CMD_EXACT, &CommonCommands::dumpCfg, F("Dump EEPROM Contents")));
  handler->registerCommand(new MenuEntry(F("heap"), CMD_EXACT, &CommonCommands::cmdHeap, F("Free heap")));
  handler->registerCommand(new MenuEntry(F("restart"), CMD_EXACT, espRestart, F("")));
  // handler->registerCommand(new MenuEntry(F("jscfg"), CMD_EXACT, printJSONConfig, F("")));
  // handler->registerCommand(new MenuEntry(F("jjj"), CMD_EXACT, testJSON, F("")));
  handler->registerCommand(new MenuEntry(F("sss"), CMD_EXACT, stopActiveExecution, F("")));
  //handler->registerCommand(new MenuEntry(F("tms"), CMD_EXACT, cmdTestMemStore, F("")));
  //handler->registerCommand(new MenuEntry(F("sertest"), CMD_BEGIN, cmdSerTest, F("")));
  handler->registerCommand(new MenuEntry(F("oled"), CMD_BEGIN, oledHandleCommand, F("oled message")));
}
