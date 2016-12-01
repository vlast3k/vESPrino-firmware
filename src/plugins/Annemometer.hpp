#ifndef ANNEM_H
#define ANNEM_H
#include "common.hpp"
#include "PropertyList.hpp"
#include "interfaces/Plugin.hpp"
#include <Timer.h>

class AnnemometerClass : public Plugin {
public:
  AnnemometerClass() ;
  void setup(MenuHandler *handler);
  const char* getName() {
    return "Annemometer";
  }
  void loop();
  static void cmdStart(const char *line);
  void cmdStartInst(const char *line);
private:
  bool started = false;
  uint8_t port;
  uint32_t lastSent = 0;
};
#endif
