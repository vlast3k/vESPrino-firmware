#ifndef _CommonCommands_H
#define _CommonCommands_H

#include "MenuHandler.hpp"
#include <Arduino.h>

class CommonCommands {
public:
  void registerCommands(MenuHandler *handler);

private:
  static void cmdHeap(char* line);
  

};

#endif
