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


void storeToEE(int address, const char *str) {
  //char tmp[30];
  //strcpy(tmp, str);
  while (*str)  EEPROM.write(address++, (byte)*(str++));
  EEPROM.write(address, 0);
  EEPROM.commit();
}

void heap(const char * str) {
  Serial << "Heap " << str << ": " << ESP.getFreeHeap() << endl;
}


