#ifndef PropertyList_h
#define PropertyList_h


#include <Arduino.h>
#include "FS.h"

class PropertyListClass : public Plugin {
public:
  PropertyListClass();
  void begin();
  void putProperty(const __FlashStringHelper *key, const char *value);
  void putProperty(const char *key, const char *value);
  char *readProperty(const __FlashStringHelper *key);
  char *readProperty(const char *key);
  bool readBoolProperty(char *key);
  long readLongProperty(const __FlashStringHelper *key);
  bool hasProperty(const char *key);
  bool hasProperty(const __FlashStringHelper *key);

  char *getArrayProperty(const __FlashStringHelper *key, int idx);
  void removeArrayProperty(const __FlashStringHelper *key);
  void putArrayProperty(const __FlashStringHelper *key, int idx, const char *value);

  void finalizeChangeFile(File &in, File &out);

  void loop() {};

private:
  String configFileName = "/vs_cfg.txt";
  String tempFileName = "/vs_cfg.tmp";
  //char buffer[200];

};

extern PropertyListClass PropertyList;

#endif
