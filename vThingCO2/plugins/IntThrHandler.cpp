#include "Timer.h"
#include "plugins\PropertyList.hpp"
#include "common.hpp"
#include <LinkedList.h>
#include <HashMap.h>
#include "interfaces\Pair.h"
#include "plugins\PowerManager.hpp"

//Timer *tmrSendValueTimer;
Timer *tmrRawRead;
uint32_t intSendValue   = 120000L;
//uint32_t intRawRead  = 15000L;
uint32_t intRawRead  = 15000L;
// void addThreshold(Thr *t) {
//   thresholds.add(t);
// }
//
// Thr* getThreshold(const char* s) {
//   for (int i=0; i < thresholds.size(); i++) {
//     if (strcmp(thresholds.get(i)->id, s) == 0) return thresholds.get(i);
//   }
//   return NULL;
// }
//
// void removeThreshold(const char* id) {
//   for (int i=0; i < thresholds.size(); i++) {
//     if (strcmp(thresholds.get(i)->id, id) == 0) {
//       thresholds.remove(i);
//       return;
//     }
//   }
// }
//
// bool checkThresholds(LinkedList<Pair *> &values) {
//   return false;
// }

void conditionalSend(bool forceSend) {
  int rtcIt = rtcMemStore.getIterations();
  //Serial << "rtcit = " << rtcIt << endl;
  //Serial.flush();
  if (rtcIt == 0) forceSend = true;
  rtcIt ++;
  if (rtcIt >= PropertyList.readLongProperty(PROP_SND_INT) / PowerManagerClass::IterationDurationS) rtcIt = 0;
  rtcMemStore.setIterations(rtcIt);

  Serial << F("\n--- DestHanlder: sendValue ---") << endl;
  if (DEBUG) Serial << F("forceSend:") << forceSend << endl;
  LinkedList<Pair *> values = LinkedList<Pair* >();
  for (int i=0; i < sensors.size(); i++)  sensors.get(i)->getData(&values);

  //bool someThresholdExceeded = checkThresholds(values);
//  if (forceSend || someThresholdExceeded) {
  wifiConnectMulti();

  if (forceSend) {
    for (int i=0; i < destinations.size(); i++) destinations.get(i)->process(values);
  //tmrSendValueTimer->Start();
  }
  wifiOff();

  for (int i=0; i < values.size(); i++)  delete  values.get(i);
  if (DEBUG) heap("");
  tmrRawRead->setInterval(PowerManagerClass::IterationDurationS * 1000);
  tmrRawRead->Start();

}

// void on_SendValue() {
//   conditionalSend(true);
// }

void onRawRead() {
  conditionalSend(false);
}

void cmdSendNow(const char* ignore) {
  conditionalSend(true);
}

void setSendInterval (const char *line) {
  int interval = 120;
  if (strchr(line, ' ')) {
    interval = atoi(strchr(line, ' ') + 1);
  }
  int iterations = std::max(1, interval / PowerManagerClass::IterationDurationS);

  PropertyList.putProperty(PROP_SND_INT, String(iterations * PowerManagerClass::IterationDurationS).c_str());
  //PropertyList.putProperty(PROP_SND_ITER, String(iterations).c_str());
  //intSendValue = (uint32_t)interval * 1000;
  //tmrSendValueTimer->setInterval(intSendValue);
  Serial << F("Send Interval (s): ") << iterations* PowerManagerClass::IterationDurationS << endl;
}

// void updateThreshold(const char *id, float value) {
//   Thr *t = getThreshold(id);
//   if (!t) return;
//   t->thr = value;
// }
//
// void setSendThreshold(const char *line) {
//   int thr = 0;
//   if (strchr(line, ' ')) {
//     thr = atoi(strchr(line, ' ') + 1);
//   }
//   PropertyList.putProperty(PROP_SND_THR, String(thr).c_str());
// //  putJSONConfig(XX_SND_THR, String(thr).c_str());
//   updateThreshold("CO2", thr);
//   Serial << F("CO2 Threshold (ppm): ") << thr << endl;
// }
//
// void setSendThresholdJson(const char *line) {
//   char key[20], value[20];
//   while (line) {
//     line = extractStringFromQuotes(line, key, sizeof(key));
//     if (!line) return;
//     line = extractStringFromQuotes(line, value, sizeof(value));
//     if (!line) return;
//     float val = atof(value);
//     updateThreshold(key, val);
//   }
// }




void setup_IntThrHandler(MenuHandler *handler) {
  if (PropertyList.hasProperty(PROP_SND_INT)) intSendValue = PropertyList.readLongProperty(PROP_SND_INT)*1000;
  else PropertyList.putProperty(PROP_SND_INT, String(intSendValue/1000).c_str());
  //if (PropertyList.hasProperty(PROP_SND_THR)) co2Threshold    = PropertyList.readLongProperty(PROP_SND_THR);
  //
  //tmrSendValueTimer = new Timer(intSendValue, on_SendValue, millis);
//  tmrRawRead     = new Timer(intRawRead, onRawRead, millis);
  tmrRawRead     = new Timer(0, onRawRead, millis);
  //tmrSendValueTimer->Start();
  //tmrRawRead->Start();
  handler->registerCommand(new MenuEntry(F("wsi"), CMD_BEGIN, setSendInterval , F("")));
  //handler->registerCommand(new MenuEntry(F("wst"), CMD_BEGIN, setSendThreshold, F("")));
  //handler->registerCommand(new MenuEntry(F("set_thr"), CMD_BEGIN, setSendThresholdJson, F("Json Array[\"key1\", \"value1\", \"key2\", \"value2\", ...]")));
  handler->registerCommand(new MenuEntry(F("sendNow"), CMD_EXACT, cmdSendNow, F("Process destinatios and send")));

  //tmrSendValueTimer->Start();
  tmrRawRead->Start();
}

void loop_IntThrHandler() {
  // if (!tmrRawRead->isEnabled()) {
  //   conditionalSend(true);
  //
  // } else {
    //tmrSendValueTimer->Update();
    tmrRawRead->Update();
  //}
}
