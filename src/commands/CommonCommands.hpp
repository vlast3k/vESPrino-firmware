#ifndef _CommonCommands_H
#define _CommonCommands_H

#include "MenuHandler.hpp"
#include "common.hpp"
#include <Arduino.h>

class CommonCommands {
public:
  void registerCommands(MenuHandler *handler);
  static void cmdHeap(const char* line);
  static void cmdDebug(const char* line);
  static void dumpCfg(const char* line);
  static void enableSlave(const char* line);
  static void factoryReset(const char *ignore);
private:

};

#endif
