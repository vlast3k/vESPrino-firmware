#include <Streaming.h>
#include "FS.h"
#include "PropertyList.hpp"
#include "common.hpp"



PropertyListClass::PropertyListClass() {
  //Serial <<"Create Property List";
  //registerPlugin(this);
  configFileName = F("/vs_cfg.txt");
  tempFileName = F("/vs_cfg.tmp");
}

void PropertyListClass::setupPropList(MenuHandler *handler) {
//  Serial <<"PropList, register commands\n";
  handler->registerCommand(new MenuEntry(F("prop_list"), CMD_EXACT, &PropertyListClass::prop_list_cfg, F("prop_list")));
  handler->registerCommand(new MenuEntry(F("prop_set"), CMD_BEGIN, &PropertyListClass::prop_set, F("prop_set \"key\" \"value\"")));
  handler->registerCommand(new MenuEntry(F("prop_jset"), CMD_BEGIN, &PropertyListClass::prop_jset, F("prop_jset \"key\"value")));
}

void PropertyListClass::prop_set(const char *line) {
  char key[50], value[200];
  line = extractStringFromQuotes(line, key, sizeof(key));
  line = extractStringFromQuotes(line, value, sizeof(value));
  PropertyList.putProperty(key, value);
}

void PropertyListClass::prop_jset(const char *line) {
  char key[50];
  line = extractStringFromQuotes(line, key, sizeof(key));
  PropertyList.putProperty(key, line);
}

bool PropertyListClass::assertInit() {
  if (!initialized) Serial << F("Property List not yet initialized\n");
  return initialized;
}

void PropertyListClass::putProperty(const __FlashStringHelper *key, const char *value) {
  putProperty(String(key).c_str(), value);
}

void PropertyListClass::putProperty(const __FlashStringHelper *key, const __FlashStringHelper *value) {
  putProperty(String(key).c_str(), String(value).c_str());
}

char *PropertyListClass::readProperty(const __FlashStringHelper *key) {
  return readProperty(String(key).c_str());
}

char *PropertyListClass::readProperty(const String &key) {
  return readProperty(key.c_str());
}

bool PropertyListClass::readBoolProperty(const __FlashStringHelper *key) {
  char ch = readProperty(key)[0];
  return !(ch == 0 || ch == '0' || ch == 'f')  ;
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

char* PropertyListClass::getArrayProperty(const __FlashStringHelper *key, int idx) {
  String _key = String(key) + idx;
  return readProperty(_key.c_str());
}

void PropertyListClass::putArrayProperty(const __FlashStringHelper *key, int idx, const char *value) {
  if (!assertInit()) return;

  if (String(key).length() == 0) return;
  String _key = String(key) + idx;
  putProperty(_key.c_str(), value);
}

void trim(char *str) {
  char *p = strchr(str, 13);
  if (p != NULL) *p = 0;
  p = strchr(str, 10);
  if (p != NULL) *p = 0;
}

void PropertyListClass::factoryReset() {
  if (!SPIFFS.remove(configFileName)) {
    Serial << F("Could not delete: ") << configFileName<< endl;
  } else {
    Serial << F("SPIFFS configuration removed\n");
  }
}

void PropertyListClass::putProperty(const char *key, const char *value) {
  if (!assertInit()) return;
  if (!key[0]) return;
  File in = SPIFFS.open(configFileName, "r");
  File out= SPIFFS.open(tempFileName, "w");
  //Serial << "Put Property: " << key << " =" << value << "." << endl;

  String _key = String(key) + "=";
  while (in.available()) {
    String line = in.readStringUntil('\n');
    line.trim();
    //Serial << "line=" << line << endl;
    if (!line.startsWith(_key)) out.println(line);
  }
  if (value[0]) out << key << "=" << value << endl;

  finalizeChangeFile(in, out);
}

char *PropertyListClass::readProperty(const char *key) {
  if (assertInit() && key[0]) {
    File in = SPIFFS.open(configFileName, "r");
    String _key = String(key) + "=";
    int r;
    while (r = in.available()) {
      String line = in.readStringUntil('\n');
      line.trim();
      if (line.startsWith(_key)) {
        strcpy(commonBuffer200, line.c_str() + _key.length());
        in.close();
        return commonBuffer200;
      }
    }
    in.close();
  }
  commonBuffer200[0] = 0;
  return commonBuffer200;
}

void PropertyListClass::removeArrayProperty(const __FlashStringHelper *key) {
  if (!assertInit()) return;

  String _key = String(key);
  if (_key.length() == 0) return;
  File in = SPIFFS.open(configFileName, "r");
  File out= SPIFFS.open(tempFileName, "w");

  while (in.available()) {
    String line = in.readStringUntil('\n');
    line.trim();
    if (!line.startsWith(_key)) out.println(line);
  }

  finalizeChangeFile(in, out);
}

void PropertyListClass::finalizeChangeFile(File &in, File &out) {
  in.close();
  out.close();

  if (!SPIFFS.remove(configFileName)) {
    Serial << F("Could not delete: ") << configFileName<< endl;
  }
  if (!SPIFFS.rename(tempFileName, configFileName)) {
    Serial << F("Could not rename ") << tempFileName << F(" to ") << configFileName << endl;
  }
}

void PropertyListClass::begin(MenuHandler *handler) {
  setupPropList(handler);
  bool res =  SPIFFS.begin();
  //Serial <<"SPFFS begin = " << res << endl;
  if (!res) {
    SPIFFS.format();
    SPIFFS.begin();
  }
  if (!SPIFFS.exists(configFileName)) {
    //Serial << "Created " << configFileName << endl;
    File f = SPIFFS.open(configFileName, "w");
    if (!f) {
      Serial << "Could not open file test" << endl;
    }
    f.close();
  }
  File f = SPIFFS.open("/tst.ttt", "w");
  //f << "vvv";
  f.close();
  if (!SPIFFS.remove("/tst.ttt")) {
    Serial << " Could not delete /tst.ttt" << endl;
  } else {
    //Serial << " successfully deleted" << endl;
  }
  initialized = true;
}

void PropertyListClass::prop_list_cfg(const char *line) {
  File in = SPIFFS.open(PropertyList.configFileName, "r");
  Serial << F("---vESPrinoCFG_start---\n");
  Serial.flush();
  delay(1);
  while (in.available())  {
    String s = in.readStringUntil('\n');
    s.trim();
    Serial << s << endl;
    Serial.flush();
    delay(1);
  }
  Serial << F("---vESPrinoCFG_end---\n");
  Serial.flush();
  delay(1);
  in.close();
}
