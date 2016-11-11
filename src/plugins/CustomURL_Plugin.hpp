#ifndef CustomURL_h
#define CustomURL_h
#include "MenuHandler.hpp"

class CustomURL_Plugin {
public:
  static void registerCommands(MenuHandler *handler);
  static void cfgGENIOT(const char *p);
  static void sndGENIOT(const char *line);


private:

};


#endif
