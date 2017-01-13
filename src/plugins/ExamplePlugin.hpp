#ifndef EXAMPLEPLUGIN_H
#define EXAMPLEPLUGIN_H
#include "common.hpp"
#include "PropertyList.hpp"
#include "interfaces/Plugin.hpp"

class ExamplePluginClass : public Plugin {
public:
  ExamplePluginClass() ;
  bool setup(MenuHandler *handler);
  const char* getName() {
    return "ExamplePlugin";
  }
  void loop();
  static void cmdStart(const char *line);
  void cmdStartInst(const char *line);
private:

};
#endif
