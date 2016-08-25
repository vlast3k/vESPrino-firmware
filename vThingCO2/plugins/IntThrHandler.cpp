#include "Timer.h"
#include "plugins\PropertyList.hpp"
#include "common.hpp"
#include <LinkedList.h>
#include <HashMap.h>
#include "interfaces\Pair.h"


//define the max size of the hashtable
//const int HASH_SIZE = 10;
//storage
//HashType<const char*,Thr*> hashRawArray[HASH_SIZE];
//handles the storage [search,retrieve,insert]
//HashMap<const char*,Thr*> hashMap = HashMap<const char*,Thr*>( hashRawArray , HASH_SIZE );

Timer *tmrSendValueTimer, *tmrRawRead;
uint32_t intSendValue   = 120000L;
uint32_t intRawRead  = 15000L;
void addThreshold(Thr *t) {
  thresholds.add(t);
}

Thr* getThreshold(const char* s) {
  for (int i=0; i < thresholds.size(); i++) {
    if (strcmp(thresholds.get(i)->id, s) == 0) return thresholds.get(i);
  }
  return NULL;
}

void removeThreshold(const char* id) {
  for (int i=0; i < thresholds.size(); i++) {
    if (strcmp(thresholds.get(i)->id, id) == 0) {
      thresholds.remove(i);
      return;
    }
  }
}

void on_SendValue() {
  Serial << "\n--- DestHanlder: sendValue ---" << endl;
  LinkedList<Pair *> values = LinkedList<Pair* >();
  for (int i=0; i < sensors.size(); i++) {
    sensors.get(i)->getData(&values);
    // const char *id = sensors.get(i)->getSensorId();
    // getThreshold(id)->oldValue = sensors.get(i)->getValue();
    // Pair *p = new Pair();
    // p->key = sensors.get(i)->getSensorId();
    // p->value = String(getThreshold(id)->oldValue);
    // Serial << getThreshold(id)->oldValue << " " << (int)getThreshold(id)->oldValue << " " << String(getThreshold(id)->oldValue) <<" " << String(getThreshold(id)->oldValue).c_str() << endl;
    // p.value = String(getThreshold(id)->oldValue).c_str();
    // Serial << p.key << " = " << p.value << " = " << sensors.get(i)->getValue() << endl;
    // p.value = String("sadsadsadsa").c_str();
    //Serial << "newval: " << p->value << "..."<<endl;
    //values.add(p);
  }
  // for (int i=0; i < values.size(); i++) {
  //   Serial << values.get(i)->key << " = " << values.get(i)->value << "."<<endl;
  // }
  for (int i=0; i < destinations.size(); i++) {
    destinations.get(i)->process(values);
  }
  // for (int i=0; i < values.size(); i++) {
  //   Serial << values.get(i)->key << " = " << values.get(i)->value << "."<<endl;
  // }
  for (int i=0; i < values.size(); i++) {
    delete  values.get(i);
  }

  heap("");

  tmrSendValueTimer->Start();
}

void onRawRead() {
  //bool shouldSend = false;
  // for (int i=0; i < sensors.size(); i++) {
  //   float currentValue = sensors.get(i)->getValue();
  //   const char* id = sensors.get(i)->getSensorId();
  //   if (getThreshold(id)->thr > 0 && abs(currentValue - getThreshold(id)->oldValue) > getThreshold(id)->thr) {
  //     on_SendValue();
  //     return;
  //   }
  // }
}

void setSendInterval (const char *line) {
  int interval = 120;
  if (strchr(line, ' ')) {
    interval = atoi(strchr(line, ' ') + 1);
  }
  PropertyList.putProperty(PROP_SND_INT, String(interval).c_str());
  intSendValue = (uint32_t)interval * 1000;
  tmrSendValueTimer->setInterval(intSendValue);
  Serial << "Send Interval (s): " << interval << endl;
}

void updateThreshold(const char *id, float value) {
  Thr *t = getThreshold(id);
  if (!t) return;
  t->thr = value;
}

void setSendThreshold(const char *line) {
  int thr = 0;
  if (strchr(line, ' ')) {
    thr = atoi(strchr(line, ' ') + 1);
  }
  PropertyList.putProperty(PROP_SND_THR, String(thr).c_str());
//  putJSONConfig(XX_SND_THR, String(thr).c_str());
  updateThreshold("CO2", thr);
  Serial << F("CO2 Threshold (ppm): ") << thr << endl;
}



void setSendThresholdJson(const char *line) {
  char key[20], value[20];
  while (line) {
    line = extractStringFromQuotes(line, key, sizeof(key));
    if (!line) return;
    line = extractStringFromQuotes(line, value, sizeof(value));
    if (!line) return;
    float val = atof(value);
    updateThreshold(key, val);
  }
}

void cmdSendNow(const char* ignore) {
  on_SendValue();
}


void setup_IntThrHandler(MenuHandler *handler) {
  if (PropertyList.hasProperty(PROP_SND_INT)) intSendValue = PropertyList.readLongProperty(PROP_SND_INT)*1000;
  //if (PropertyList.hasProperty(PROP_SND_THR)) co2Threshold    = PropertyList.readLongProperty(PROP_SND_THR);
  tmrSendValueTimer = new Timer(intSendValue, on_SendValue);
  tmrRawRead     = new Timer(intRawRead,   onRawRead);
  tmrSendValueTimer->Start();
  tmrRawRead->Start();
  handler->registerCommand(new MenuEntry(F("wsi"), CMD_BEGIN, setSendInterval , F("")));
  handler->registerCommand(new MenuEntry(F("wst"), CMD_BEGIN, setSendThreshold, F("")));
  handler->registerCommand(new MenuEntry(F("set_thr"), CMD_BEGIN, setSendThresholdJson, F("Json Array[\"key1\", \"value1\", \"key2\", \"value2\", ...]")));
  handler->registerCommand(new MenuEntry(F("sendNow"), CMD_EXACT, cmdSendNow, F("Process destinatios and send")));
}

void loop_IntThrHandler() {
  tmrSendValueTimer->Update();
  tmrRawRead->Update();
}
