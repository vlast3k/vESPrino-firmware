#ifndef PropertyList_h
#define PropertyList_h

#include <Arduino.h>
class PropertyListClass {
public:
  void begin();
  void putProperty(const __FlashStringHelper *key, char *value);
  void putProperty(const char *key, char *value);
  char *readProperty(const __FlashStringHelper *key, char *value);
  char *readProperty(const char *key, char *value);
  bool readBoolProperty(char *key);



private:
  String configFileName = "/vs_cfg.txt";
  String tempFileName = "/vs_cfg.tmp";

};

extern PropertyListClass PropertyList;

#endif
