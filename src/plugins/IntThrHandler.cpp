#include "Timer.h"
#include "plugins/PropertyList.hpp"
#include "common.hpp"
#include <LinkedList.h>
#include "interfaces/Pair.h"
#include "plugins/PowerManager.hpp"
#include "plugins/NeopixelVE.hpp"
extern NeopixelVE neopixel;
#include "plugins/TimerManager.hpp"
extern TimerManagerClass TimerManager;


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
void addCommonValues(LinkedList<Pair *> *data) {
  data->add(new Pair("IP", String(WiFi.localIP().toString())));
  String chipId = String(ESP.getChipId(), HEX);
  chipId.toUpperCase();
  data->add(new Pair("CHIPID", chipId));
  data->add(new Pair("RUNTIME", String((millis() + rtcMemStore.getGenData(GEN_MSCOUNTER)) / 1000)));
  data->add(new Pair("GMTTIME", String(TimerManager.getGMTime())));
}

void conditionalSend(bool forceSend) {
  PERF("SEND 1")
  yield();
  int rtcIt = rtcMemStore.getIterations();
  //LOGGER << "rtcit = " << rtcIt << endl;
  //LOGGER.flush();
  if (rtcIt == 0) forceSend = true;
  rtcIt ++;
  if (rtcIt >= PropertyList.readLongProperty(PROP_SND_INT) / PowerManagerClass::IterationDurationS) rtcIt = 0;
  rtcMemStore.setIterations(rtcIt);
  LinkedList<Pair *> values = LinkedList<Pair* >();
  addCommonValues(&values);
  for (int i=0; i < sensors.size(); i++)  {
    yield();
    if (storedSensors.indexOf(sensors.get(i)->getName()) > -1) {
      uint32_t x = millis();
      sensors.get(i)->getData(&values);
      x = millis() - x;
      LOGGER << sensors.get(i)->getName() << " : " <<  x << F("ms") << endl;
    }
  }

  PERF("SEND 2")

  //bool someThresholdExceeded = checkThresholds(values);
//  if (forceSend || someThresholdExceeded) {
//  wifiConnectMulti();
  yield();

  bool res = true;
  if (forceSend) {
    LOGGER << F("\n---Sending data---") << endl;
    LOGGER.flush();
    yield();
    for (int i=0; i < destinations.size(); i++) {
      uint32_t x = millis();
      res = destinations.get(i)->process(values) && res;
      x = millis() - x;
      LOGGER << destinations.get(i)->getName() << " : " <<  x << F("ms") << endl;
    }
    if (res) neopixel.signal(LED_SEND_OK);
    else neopixel.signal(LED_SEND_FAILED);
  //tmrSendValueTimer->Start();
  }
//  wifiOff();

  for (int i=0; i < values.size(); i++)  delete  values.get(i);
  if (DEBUG) heap("");
  tmrRawRead->setInterval(PowerManagerClass::IterationDurationS * 1000);
  tmrRawRead->Start();
  PERF("SEND 3")
  LOGGER.flushLog();

}

// void on_SendValue() {
//   conditionalSend(true);
// }

void onRawRead() {
  menuHandler.scheduleCommand("@sendNowCond");
  //conditionalSend(false);
}

void cmdSendNow(const char* ignore) {
  conditionalSend(true);
}

void cmdSendNowCond(const char* ignore) {
  conditionalSend(false);
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
  LOGGER << F("Send Interval (s): ") << iterations* PowerManagerClass::IterationDurationS << endl;
  LOGGER.flush();
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
//   LOGGER << F("CO2 Threshold (ppm): ") << thr << endl;
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
  handler->registerCommand(new MenuEntry(F("sendNowCond"), CMD_EXACT, cmdSendNowCond, F("Process destinatios and send")));

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
