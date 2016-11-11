#ifndef Pair_h
#define Pair_h
#include <Arduino.h>
struct Pair {
  Pair (const char *_key, String _value) : key(_key), value(_value) {};
  const char *key;
  String value;
};

struct Thr {
  Thr (const char* _id, float _thr) {
    strcpy(id, _id);
    thr = _thr;
  }
  float oldValue = 0;
  float thr = 0;
  char id[7];
};

#endif
