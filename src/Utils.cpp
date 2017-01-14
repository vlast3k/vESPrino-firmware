#include <Arduino.h>
#include <algorithm>    // std::min
#include "common.hpp"

#define PROP_DECIMAL_SEPARATOR F("decimal.separator")

char *extractStringFromQuotes(const char* src, char *dest, int destSize) {
  if (!*src) {
    *dest = 0;
    return NULL;
  }
  char *p1, *p2;
  if (!(p1 = strchr(src, '\"'))) return 0;
  if (!(p2 = strchr(p1+1, '\"'))) return 0;
  int len = std::min(p2-p1, destSize);
  strncpy(dest, p1+1, len-1);
  dest[len-1] = 0;
  return p2 + 1;
}


void storeToEE(int address, const char *str, int maxLength) {
  //char tmp[30];
  //strcpy(tmp, str);
  while (*str && --maxLength)  EEPROM.write(address++, (byte)*(str++));
  EEPROM.write(address, 0);
  EEPROM.commit();
}

void heap(const char * str) {
  LOGGER << "Heap " << str << ": " << ESP.getFreeHeap() << endl;
}

char *getJSONConfig(const char *item, char *dest, char *p2, char *p3) {
  StaticJsonBuffer<400> jsonBuffer;
  char data[1000];
  dest[0] = 0;
  EEPROM.get(EE_JSON_CFG_1000B, data);
  //LOGGER << "JSON cfg: " << data << endl;
  if (data[0] == -1 || data[0] == 0 || data[0] == 255) strcpy(data, "{}");
  JsonObject& root = jsonBuffer.parseObject(data);
  if (!root.success() || !root.containsKey(item)) return dest;
  if (root[item].is<const char*>()) {
    strcpy(dest, root[item].asString());
  } else {
    if (root[item][0].asString()) strcpy(dest, root[item][0].asString());
    if (root[item][1].asString()) strcpy(p2  , root[item][1].asString());
    if (root[item][2].asString()) strcpy(p3  , root[item][2].asString());
  }
  //LOGGER << "dest: " << dest << endl;
  return dest;
}
void putJSONConfig(const char *key, int value, boolean commit) {
  putJSONConfig(key, String(value).c_str(), false, commit);
}



void putJSONConfig(const char *key, const char *value, boolean valueIsArray, boolean commit) {
  StaticJsonBuffer<600> jsonBuffer;
  char data2[1000], data[1000];

  EEPROM.get(EE_JSON_CFG_1000B, data);
  if (data[0] == -1 || data[0] == 0 || data[0] == 255) strcpy(data, "{}");
  JsonObject& root = jsonBuffer.parseObject(data);
  char p1[50] = {0}, p2[50] = {0}, p3[50] = {0};
  if (!valueIsArray) {
    root[key] = value;
  } else {
    value = extractStringFromQuotes(value, p1, sizeof(p1));
    if (value) value = extractStringFromQuotes(value, p2, sizeof(p2));
    if (value) value = extractStringFromQuotes(value, p3, sizeof(p3));
    JsonArray& data = root.createNestedArray(key);
    data.add(p1);
    if (p2[0]) data.add(p2);
    if (p3[0]) data.add(p3);

  }
  root.printTo(data2, sizeof(data2));
  EEPROM.put(EE_JSON_CFG_1000B, data2);
  if (commit) EEPROM.commit();
}

// void testJSON(const char *ignore) {
//   char ddd[1000] = "";
//   char tmp[100];
//   EEPROM.put(EE_JSON_CFG_1000B, ddd);
//   EEPROM.commit();
//
//   EEPROM.write(EE_JSON_CFG_1000B, -1);
//   EEPROM.commit();
//   LOGGER << "Testing JSON" << endl;
//   LOGGER << getJSONConfig("vladi", tmp) << endl;
//   putJSONConfig("vladi", "sadsda");
//   LOGGER << "1 " << getJSONConfig("vladi", tmp) << endl;
//   putJSONConfig("vladi", "sadsa");
//   LOGGER << "2 " << getJSONConfig("vladi", tmp) << endl;
//   LOGGER << endl;
//   EEPROM.write(EE_JSON_CFG_1000B, -1);
//   EEPROM.commit();
// }
//
 void printJSONConfig(const char* ignore) {
  StaticJsonBuffer<400> jsonBuffer;
  char data[1000];
  EEPROM.get(EE_JSON_CFG_1000B, data);
  if (data[0] == -1 || data[0] == 0) strcpy(data, "{}");
  JsonObject& root = jsonBuffer.parseObject(data);
  root.printTo(LOGGER);
 }



void dumpArray(const char *s) {
  while (*s) LOGGER << _HEX(*(s++)) << ",";
  LOGGER << endl;
}

char* getListItem(const char* str, char *buf, int idx, char sep) {
  for (int i=0; i < idx && (str = strchr(str, sep)) != NULL; i++, str++);
  if (str == NULL) return NULL;
  char *e = strchr(str, sep);
  int len = (e == NULL)? strlen(str) : e-str;
  strncpy(buf, str, len);
  buf[len] = 0;
  return buf;
}

int getListItemCount(const char* str) {
  char buf[200];
  if (!*str) return 0;
  char *p;
  int i=0;
  for (; (p = getListItem(str, buf, i)) != NULL; i++);
  return i;
}

char decimalSeparator = 0;
void initDecimalSeparator() {
  decimalSeparator = PropertyList.readProperty(PROP_DECIMAL_SEPARATOR)[0];
  if (!decimalSeparator) decimalSeparator = '.';
}

void replaceDecimalSeparator(String &src) {
  src.replace('.', decimalSeparator);
}
