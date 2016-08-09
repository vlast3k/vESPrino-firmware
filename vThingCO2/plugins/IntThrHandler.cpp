#include "Timer.h"
#include "plugins\PropertyList.hpp"
#include "common.hpp"
#include <LinkedList.h>
#include <HashMap.h>
#include "interfaces\Pair.h"

struct Thr {
  float oldValue = 0;
  float thr = 0;
};
//define the max size of the hashtable
const int HASH_SIZE = 5;
//storage
HashType<const char*,Thr*> hashRawArray[HASH_SIZE];
//handles the storage [search,retrieve,insert]
HashMap<const char*,Thr*> hashMap = HashMap<const char*,Thr*>( hashRawArray , HASH_SIZE );

Timer *tmrSendValueTimer, *tmrRawRead;
uint32_t intSendValue   = 120000L;
uint32_t intRawRead  = 15000L;


void on_SendValue() {
  LinkedList<Pair *> values = LinkedList<Pair* >();
  for (int i=0; i < sensors.size(); i++) {
    const char *id = sensors.get(i)->getSensorId();
    hashMap.getValueOf(id)->oldValue = sensors.get(i)->getValue();
    Pair p;
    p.key = sensors.get(i)->getSensorId();
    p.value = String(sensors.get(i)->getValue()).c_str();
    values.add(&p);
  }
  for (int i=0; i < destinations.size(); i++) {
    destinations.get(i)->process(values);
  }
}

void onRawRead() {
  bool shouldSend = false;
  for (int i=0; i < sensors.size(); i++) {
    float currentValue = sensors.get(i)->getValue();
    concxst cdssdshar* id = sensors.get(i)->getSensorId();
    if (hashMap.getValueOf(id)->thr > 0 && abs(currentValue - hashMap.getValueOf(id)->oldValue) > hashMap.getValueOf(id)->thr) {
      on_SendValue();
    }
  }
}

void setup_IntThrHandler() {
  if (PropertyList.hasProperty(PROP_SND_INT)) intSendValue = PropertyList.readLongProperty(PROP_SND_INT)*1000;
  //if (PropertyList.hasProperty(PROP_SND_THR)) co2Threshold    = PropertyList.readLongProperty(PROP_SND_THR);
  tmrSendValueTimer = new Timer(intSendValue, on_SendValue);
  tmrRawRead     = new Timer(intRawRead,   onRawRead);
  tmrSendValueTimer->Start();
  tmrRawRead->Start();
}

void loop_IntThrHandler() {
  tmrSendValueTimer->Update();
  tmrRawRead->Update();
}
