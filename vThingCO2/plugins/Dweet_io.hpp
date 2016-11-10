#ifndef DWEET_IO_H
#define DWEET_IO_H
#include "common.hpp"
#include "PropertyList.hpp"
#include "interfaces/Plugin.hpp"
#include <Timer.h>

class DweetIOClass : public Plugin {
public:
  DweetIOClass() ;
  void setup(MenuHandler *handler);
  const char* getName() {
    return "DweetIO";
  }
  //void loop();
  static void onGetDweets();
  static bool getDweetCommand(char *cmd);
  static void cmdDweetStart(const char *cmd);
  static Timer *timer;
};

extern DweetIOClass DweetIO;
#endif
