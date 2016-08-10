#include <Streaming.h>
#include "FS.h"
#include "PropertyList.hpp"
#include "common.hpp"

void PropertyListClass::begin() {
  bool res =  SPIFFS.begin();
  if (!res) {
    SPIFFS.format();
    SPIFFS.begin();
  }
  if (!SPIFFS.exists(configFileName)) {
    File f = SPIFFS.open(configFileName, "w");
    if (!f) {
      Serial << "Could not open file test" << endl;
    }
    f.close();
  }
}


void PropertyListClass::putProperty(const __FlashStringHelper *key, const char *value) {
  putProperty(String(key).c_str(), value);
}
void PropertyListClass::putProperty(const char *key, const char *value) {
  File in = SPIFFS.open(configFileName, "r");
  File out= SPIFFS.open(tempFileName, "w");
  String _key = String(key) + "=";
  while (in.available()) {
    String line = in.readStringUntil('\n');
    if (!line.startsWith(_key)) out.println(line);
  }
  out << key << "=" << value << endl;
  in.close();
  out.close();

  if (!SPIFFS.remove(configFileName)) {
    Serial << "Could not delete: " << configFileName<< endl;
  }
  if (!SPIFFS.rename(tempFileName, configFileName)) {
    Serial << "Could not rename " << tempFileName << " to " << configFileName << endl;
  }
}

char *PropertyListClass::readProperty(const __FlashStringHelper *key) {
  return readProperty(String(key).c_str());
}

char *PropertyListClass::readProperty(const char *key) {
  File in = SPIFFS.open(configFileName, "r");
  String _key = String(key) + "=";
  int r;
  while (r = in.available()) {
    String line = in.readStringUntil('\n');
    if (line.startsWith(_key)) {
      strcpy(commonBuffer200, line.c_str() + _key.length());
      return commonBuffer200;
    }
  }
  commonBuffer200[0] = 0;
  return commonBuffer200;
}

bool PropertyListClass::readBoolProperty(char *key) {
  return readProperty(key)[0] != 0;
}

bool PropertyListClass::hasProperty(const __FlashStringHelper *key) {
  return readProperty(key)[0] != 0;
}

bool PropertyListClass::hasProperty(const char *key) {
  return readProperty(key)[0] != 0;
}

long PropertyListClass::readLongProperty(const __FlashStringHelper *key) {
  if (!hasProperty(key)) return 0;
  return atol(readProperty(key));

}

PropertyListClass PropertyList;
