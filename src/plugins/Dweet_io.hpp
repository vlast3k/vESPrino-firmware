#ifndef DWEET_IO_H
#define DWEET_IO_H
#include "common.hpp"
#include "PropertyList.hpp"
#include "interfaces/Plugin.hpp"
#include <Timer.h>

class DweetIOClass : public Plugin {
public:
  DweetIOClass() ;
  bool setup(MenuHandler *handler);
  const char* getName() {
    return "DweetIO";
  }
  //void loop();
  static void onGetDweets();
  bool getDweetCommand(char *cmd);
  static void cmdDweetStart(const char *cmd);
  void cmdDweetStartInst(const char *cmd);
  static void cmdDweetProcess(const char *cmd);
  void cmdDweetProcessInst(const char *cmd);

  Timer *timer;
  bool isAcceptStoredDweets;
  String dwKey;
};

extern DweetIOClass DweetIO;
#endif
