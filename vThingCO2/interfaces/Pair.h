#ifndef Pair_h
#define Pair_h
#include <Arduino.h>
struct Pair {
  const char *key;
  const char *value;
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
