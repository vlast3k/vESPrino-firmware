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
  SERIAL << "Heap " << str << ": " << ESP.getFreeHeap() << endl;
}

String getJSONConfig(const char *item) {
  StaticJsonBuffer<200> jsonBuffer;
  char data[1000];
  EEPROM.get(EE_JSON_CFG_1000B, data);
  //SERIAL << "JSON cfg: " << data << endl;
  if (data[0] == -1 || data[0] == 0 || data[0] == 255) strcpy(data, "{}");
  JsonObject& root = jsonBuffer.parseObject(data);
  return String(root[item].asString());
}
void putJSONConfig(const char *key, int value, boolean commit) {
  putJSONConfig(key, String(value).c_str(), commit);
}

void putJSONConfig(const char *key, const char *value, boolean commit) {
  StaticJsonBuffer<200> jsonBuffer;
  char data2[1000], data[1000];

  EEPROM.get(EE_JSON_CFG_1000B, data);
  if (data[0] == -1 || data[0] == 0 || data[0] == 255) strcpy(data, "{}");
  JsonObject& root = jsonBuffer.parseObject(data);
  
  root[key] = value;
  root.printTo(data2, sizeof(data2));
  EEPROM.put(EE_JSON_CFG_1000B, data2);
  if (commit) EEPROM.commit();  
}

void testJSON() {
  char ddd[1000] = "";
  EEPROM.put(EE_JSON_CFG_1000B, ddd);
  EEPROM.commit();
  
  EEPROM.write(EE_JSON_CFG_1000B, -1);
  EEPROM.commit();
  SERIAL << "Testing JSON" << endl;
  SERIAL << getJSONConfig("vladi") << endl;
  putJSONConfig("vladi", "sadsa");
  SERIAL << "1 " << getJSONConfig("vladi") << endl;  
  putJSONConfig("vladi", "sadsa");
  SERIAL << "2 " << getJSONConfig("vladi") << endl;  
  SERIAL << endl;
  EEPROM.write(EE_JSON_CFG_1000B, -1);
  EEPROM.commit();
}

 void printJSONConfig() {
  StaticJsonBuffer<200> jsonBuffer;
  char data[1000];
  EEPROM.get(EE_JSON_CFG_1000B, data);
  if (data[0] == -1 || data[0] == 0) strcpy(data, "{}");
  JsonObject& root = jsonBuffer.parseObject(data);
  root.printTo(SERIAL);
 }

