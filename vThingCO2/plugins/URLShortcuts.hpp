#ifndef URLShortcuts_h
#define URLShortcuts_h
#include "MenuHandler.hpp"
class URLShortcuts {
public:
  static void registerCommands(MenuHandler *handler);
  static void testUBI(const char *line);
  static void sendTS(const char *line);
  static void sndSimple(const char *line);
  static void doTest(const char *line);
  static void setTSKey(const char *line);
  static void setUBIKey(const char *line);
  static void setUBIValue(const char *line);

private:

};

#endif
