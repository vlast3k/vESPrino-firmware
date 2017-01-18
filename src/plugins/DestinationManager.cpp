#include "plugins/DestinationManager.hpp"
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
extern DestinationManagerClass DestinationManager;

Timer *tmrRawRead;
uint32_t intSendValue   = 120000L;

DestinationManagerClass::DestinationManagerClass() {
  registerPlugin(this);
}

void DestinationManagerClass::onProperty(String &key, String &value) {
  if (key == PROP_SND_INT) intSendValue = atol(value.c_str()) * 1000;
}

bool DestinationManagerClass::setup(MenuHandler *handler) {
  tmrRawRead     = new Timer(0, DestinationManagerClass::onRawRead);
  handler->registerCommand(new MenuEntry(F("wsi"), CMD_BEGIN, &DestinationManagerClass::setSendInterval , F("")));
  handler->registerCommand(new MenuEntry(F("sendNow"), CMD_EXACT, &DestinationManagerClass::cmdSendNow, F("Process destinatios and send")));
  handler->registerCommand(new MenuEntry(F("sendNowCond"), CMD_EXACT, &DestinationManagerClass::cmdSendNowCond, F("Process destinatios and send")));
  tmrRawRead->Start();
  return false;
}

void DestinationManagerClass::loop() {
  tmrRawRead->Update();
}

void DestinationManagerClass::cmdStart(const char *line) {
  DestinationManager.cmdStartInst(line);
}

void DestinationManagerClass::cmdStartInst(const char *line) {
}

void DestinationManagerClass::addCommonValues(LinkedList<Pair *> *data) {
  data->add(new Pair("IP", String(WiFi.localIP().toString())));
  String chipId = String(ESP.getChipId(), HEX);
  chipId.toUpperCase();
  data->add(new Pair("CHIPID", chipId));
  data->add(new Pair("RUNTIME", String((millis() + rtcMemStore.getGenData(GEN_MSCOUNTER)) / 1000)));
  data->add(new Pair("GMTTIME", String(TimerManager.getGMTime())));
  data->add(new Pair("RSSI", String(WiFi.RSSI())));
}

void DestinationManagerClass::onIterationStart() {
  int rtcIt = rtcMemStore.getIterations();
  if (rtcIt == 0) willSendThisIteration = true;
  rtcIt ++;
  if (rtcIt >= (intSendValue/1000) / PowerManagerClass::IterationDurationS) rtcIt = 0;
  rtcMemStore.setIterations(rtcIt);
}
void DestinationManagerClass::readSensorValues(LinkedList<Pair *> *values) {
  for (int i=0; i < sensors.size(); i++)  {
    yield();
    if (storedSensors.indexOf(sensors.get(i)->getName()) > -1) {
      uint32_t x = millis();
      sensors.get(i)->getData(values);
      x = millis() - x;
      LOGGER << sensors.get(i)->getName() << " : " <<  x << F("ms") << endl;
    }
  }
}

bool DestinationManagerClass::sendDataToDestinations(LinkedList<Pair *> *values) {
  bool res = true;
  LOGGER << F("\n---Sending data---") << endl;
  LOGGER.flush();
  yield();
  for (int i=0; i < destinations.size(); i++) {
    uint32_t x = millis();
    res = destinations.get(i)->process(*values) && res;
    x = millis() - x;
    LOGGER << destinations.get(i)->getName() << " : " <<  x << F("ms") << endl;
  }
  return res;
}

void DestinationManagerClass::conditionalSend(bool forceSend) {
  PERF("SEND 1")
  yield();
  forceSend = forceSend || getWillSendThisIteration();
  if (!forceSend) return;
  LinkedList<Pair *> values = LinkedList<Pair* >();
  addCommonValues(&values);
  readSensorValues(&values);
  PERF("SEND 2")
  yield();
  bool res = true;
  if (forceSend) res = sendDataToDestinations(&values);
  if (res) neopixel.signal(LED_SEND_OK);
  else neopixel.signal(LED_SEND_FAILED);


  for (int i=0; i < values.size(); i++)  delete  values.get(i);

  if (DEBUG) heap("");
  tmrRawRead->setInterval(intSendValue);
  tmrRawRead->Start();
  PERF("SEND 3")
  LOGGER.flushLog();
}

// void on_SendValue() {
//   conditionalSend(true);
// }

void DestinationManagerClass::onRawRead() {
  menuHandler.scheduleCommand("@sendNow");
  //conditionalSend(false);
}

void DestinationManagerClass::cmdSendNow(const char* ignore) {
  DestinationManager.conditionalSend(true);
}

void DestinationManagerClass::cmdSendNowCond(const char* ignore) {
  DestinationManager.conditionalSend(false);
}

void DestinationManagerClass::setSendInterval(const char *line) {
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
