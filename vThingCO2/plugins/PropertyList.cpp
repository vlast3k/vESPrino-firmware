#include <Streaming.h>
#include "FS.h"
#include "PropertyList.hpp"

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

char *PropertyListClass::readProperty(const __FlashStringHelper *key, char *value) {
  return readProperty(String(key).c_str(), value);
}

char *PropertyListClass::readProperty(const char *key, char *value) {
  File in = SPIFFS.open(configFileName, "r");
  String _key = String(key) + "=";
  int r;
  while (r = in.available()) {
    String line = in.readStringUntil('\n');
    if (line.startsWith(_key)) {
      strcpy(value, line.c_str() + _key.length());
      return value;
    }
  }
  value[0] = 0;
  return value;
}

bool PropertyListClass::readBoolProperty(char *key) {
  char v[10];
  return readProperty(key, v)[0] != 0;
}

bool PropertyListClass::hasProperty(const __FlashStringHelper *key) {
  char v[200];
  return readProperty(key, v)[0] != 0;
}

long PropertyListClass::readLongProperty(const __FlashStringHelper *key) {
  if (!hasProperty(key)) return 0;
  char v[20];
  return atol(readProperty(key, v));

}

PropertyListClass PropertyList;
