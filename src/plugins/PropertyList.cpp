#include <Streaming.h>
#include "FS.h"
#include "PropertyList.hpp"
#include "common.hpp"

void reportProperty(String &key, String &value);

PropertyListClass::PropertyListClass() {
  //LOGGER <<"Create Property List";
  //registerPlugin(this);
  configFileName = F("/vs_cfg.txt");
  tempFileName = F("/vs_cfg.tmp");
}

void PropertyListClass::setupPropList(MenuHandler *handler) {
//  LOGGER <<"PropList, register commands\n";
  handler->registerCommand(new MenuEntry(F("prop_list"), CMD_EXACT, &PropertyListClass::prop_list_cfg, F("prop_list")));
  handler->registerCommand(new MenuEntry(F("format_spiffs"), CMD_EXACT, &PropertyListClass::format_spiffs, F("format_spiffs")));
  handler->registerCommand(new MenuEntry(F("prop_set"), CMD_BEGIN, &PropertyListClass::prop_set, F("prop_set \"key\" \"value\"")));
  handler->registerCommand(new MenuEntry(F("prop_jset"), CMD_BEGIN, &PropertyListClass::prop_jset, F("prop_jset \"key\"value")));
}

void PropertyListClass::format_spiffs(const char *line) {
  SPIFFS.format();
  SPIFFS.begin();
  PropertyList.begin(&menuHandler);
  LOGGER << F("FS formated");

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
  if (!initialized) {
    beginInt();
    initialized = true;
  }
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

bool PropertyListClass::toBool(String &str) {
  char ch = str.charAt(0);
//  ..Serial << "ProprlisttoBool: ch=" << ch << endl;
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
  beginInt();
  if (!SPIFFS.remove(configFileName)) {
    LOGGER << F("Could not delete: ") << configFileName<< endl;
  } else {
    LOGGER << F("SPIFFS configuration removed\n");
  }
  File f = SPIFFS.open(configFileName, "w");
  if (!f) {
    LOGGER << F("Could not open file test") << endl;
  }
  f.close();
  endInt();
}

void PropertyListClass::putProperty(const char *key, const char *value) {
  // uint32_t st = millis();
  if (!key[0]) {
    endInt();
  }
  if (!assertInit()) return;

  File in = SPIFFS.open(configFileName, "r");
  File out= SPIFFS.open(tempFileName, "w");

  // LOGGER << "Put Property start: " << (millis() - st) << endl;
  // LOGGER.flush();
  // st = millis();

  String _key = String(key) + "=";
  while (in.available()) {
    String line = in.readStringUntil('\n');
    line.trim();
    //LOGGER << "line=" << line << endl;
    if (!line.startsWith(_key)) out.println(line);
  }
  if (value[0]) out << key << "=" << value << endl;
  String skey = key;
  String sval = value;
  reportProperty(skey, sval);

  // LOGGER << "Put Property set: " << (millis() - st) << endl;
  // LOGGER.flush();
  // st = millis();

  finalizeChangeFile(in, out);
  endInt();
  // LOGGER << "Put Property write: " << (millis() - st) << endl;
  // LOGGER.flush();
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
        endInt();
        return commonBuffer200;
      }
    }
    in.close();
  }
  commonBuffer200[0] = 0;
  endInt();
  return commonBuffer200;
}

void PropertyListClass::reportProperties() {
  if (assertInit()) {
    File in = SPIFFS.open(configFileName, "r");
    int r;
    while (r = in.available()) {
      String line = in.readStringUntil('\n');
      int e = line.indexOf('=');
      if (e > -1) {
        String key = line.substring(0, e);
        String value = line.substring(e+1, line.length());
        key.trim();
        value.trim();
        reportProperty(key, value);
      }
    }
    in.close();
  }
  endInt();
}

void PropertyListClass::removeArrayProperty(const __FlashStringHelper *key) {
  if (!assertInit()) return;

  String _key = String(key);
  if (_key.length() == 0) {
    endInt();
    return;
  }
  File in = SPIFFS.open(configFileName, "r");
  File out= SPIFFS.open(tempFileName, "w");

  while (in.available()) {
    String line = in.readStringUntil('\n');
    line.trim();
    if (!line.startsWith(_key)) out.println(line);
  }

  finalizeChangeFile(in, out);
  endInt();
}

void PropertyListClass::finalizeChangeFile(File &in, File &out) {
  in.close();
  out.close();

  if (!SPIFFS.remove(configFileName)) {
    LOGGER << F("Could not delete: ") << configFileName<< endl;
  }
  if (!SPIFFS.rename(tempFileName, configFileName)) {
    LOGGER << F("Could not rename ") << tempFileName << F(" to ") << configFileName << endl;
  }
}

int PropertyListClass::beginInt() {
  return SPIFFS.begin();
}

void PropertyListClass::endInt() {
  SPIFFS.end();
  initialized = false;
}

void PropertyListClass::begin(MenuHandler *handler) {
  int res = beginInt();
  setupPropList(&menuHandler);

  //LOGGER <<"SPFFS begin = " << res << endl;
  if (!res) {
    SPIFFS.format();
    SPIFFS.begin();
  }
  if (!SPIFFS.exists(configFileName)) {
    //LOGGER << "Created " << configFileName << endl;
    File f = SPIFFS.open(configFileName, "w");
    if (!f) {
      LOGGER << F("Could not open file test") << endl;
    }
    f.close();
  }
  File f = SPIFFS.open("/tst.ttt", "w");
  //f << "vvv";
  f.close();
  if (!SPIFFS.remove("/tst.ttt")) {
    LOGGER << F(" Could not delete /tst.ttt") << endl;
  } else {
    //LOGGER << " successfully deleted" << endl;
  }
  //initialized = true;
  endInt();
}

void PropertyListClass::prop_list_cfg(const char *line) {
  PropertyList.beginInt();
  File in = SPIFFS.open(PropertyList.configFileName, "r");
  LOGGER << F("---vESPrinoCFG_start---\n");
  LOGGER.flush();
  delay(1);
  while (in.available())  {
    String s = in.readStringUntil('\n');
    s.trim();
    LOGGER << s << endl;
    LOGGER.flush();
    delay(1);
  }
  LOGGER << F("---vESPrinoCFG_end---\n");
  LOGGER.flush();
  delay(1);
  in.close();
  PropertyList.endInt();
}
