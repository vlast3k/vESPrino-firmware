#ifndef PropertyList_h
#define PropertyList_h


#include <Arduino.h>
#include "FS.h"
#include "interfaces\Plugin.hpp"
#include "MenuHandler.hpp"

class PropertyListClass : public Plugin {
public:
  PropertyListClass();
  void setup(MenuHandler *handler);
  void begin();
  void putProperty(const __FlashStringHelper *key, const char *value);
  void putProperty(const char *key, const char *value);
  char *readProperty(const __FlashStringHelper *key);
  char *readProperty(const char *key);
  bool readBoolProperty(const __FlashStringHelper *key);
  long readLongProperty(const __FlashStringHelper *key);
  bool hasProperty(const char *key);
  bool hasProperty(const __FlashStringHelper *key);

  char *getArrayProperty(const __FlashStringHelper *key, int idx);
  void removeArrayProperty(const __FlashStringHelper *key);
  void putArrayProperty(const __FlashStringHelper *key, int idx, const char *value);
  String configFileName;// = F("/vs_cfg.txt");
  String tempFileName;// = F("/vs_cfg.tmp");
  char* getName() {
    return "PropertyList";
  }


private:
  void finalizeChangeFile(File &in, File &out);
  void loop() {};
  static void prop_list_cfg(const char *line);
  static void prop_set(const char *line);
  void trim(char *str);
  //char buffer[200];

};

extern PropertyListClass PropertyList;

#endif
