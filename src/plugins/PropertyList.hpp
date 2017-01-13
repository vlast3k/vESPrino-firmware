#ifndef PropertyList_h
#define PropertyList_h

#define SPIFFS_CACHE 0


#include <Arduino.h>
#include "FS.h"
#include "interfaces/Plugin.hpp"
#include "MenuHandler.hpp"

class PropertyListClass {
public:
  PropertyListClass();
  int beginInt();
  void endInt();
  void begin(MenuHandler *handler);
  void putProperty(const __FlashStringHelper *key, const char *value);
  void putProperty(const __FlashStringHelper *key, const __FlashStringHelper *value);
  void putProperty(const char *key, const char *value);
  char *readProperty(const __FlashStringHelper *key);
  char *readProperty(const char *key);
  char *readProperty(const String &key);
  bool readBoolProperty(const __FlashStringHelper *key);
  long readLongProperty(const __FlashStringHelper *key);
  bool hasProperty(const char *key);
  bool hasProperty(const __FlashStringHelper *key);
  void reportProperties();
  char *getArrayProperty(const __FlashStringHelper *key, int idx);
  void removeArrayProperty(const __FlashStringHelper *key);
  void putArrayProperty(const __FlashStringHelper *key, int idx, const char *value);
  void factoryReset();
  static bool toBool(String &str);

  String configFileName;// = F("/vs_cfg.txt");
  String tempFileName;// = F("/vs_cfg.tmp");
  const char* getName() {
    return "PropertyList";
  }


private:
  void setupPropList(MenuHandler *handler);
  void finalizeChangeFile(File &in, File &out);
  void loop() {};
  static void prop_list_cfg(const char *line);
  static void prop_set(const char *line);
  static void prop_jset(const char *line);
  static void format_spiffs(const char *line);
  void trim(char *str);
  bool initialized = false;
  bool assertInit();
  //char buffer[200];

};

extern PropertyListClass PropertyList;

#endif
