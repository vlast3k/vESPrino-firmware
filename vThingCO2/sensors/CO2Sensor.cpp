//#ifdef VTHING_CO2

#include "Arduino.h"
#include "Timer.h"
#include "CubicGasSensors.h"
#include "common.hpp"
#include "plugins\PropertyList.hpp"
#include "EEPROM.h"
uint32_t intCO2RawRead   = 15000L;
uint32_t intCO2SendValue = 120000L;
uint16_t co2Threshold = 1;
uint32_t lastSentCO2value = 0;

Timer *tmrCO2RawRead, *tmrCO2SendValueTimer;
boolean startedCO2Monitoring = false;
//RunningAverage raCO2Raw(4);



void onCo2Status(CubicStatus status) {
  //SERIAL << "status = " << status << endl;
  if (status == CB_CALIBRATE) {
   // SERIAL << "status11111 = " << status << endl;
    strip->SetPixelColor(0, RgbColor(0, 0,30));
    tmrStopLED->Stop();
  } else if (status == CB_WARMUP) {
    strip->SetPixelColor(0, RgbColor(30, 30, 0));
  } else if (status == CB_STARTED) {
    strip->SetPixelColor(0, RgbColor(0, 30, 0));
    tmrStopLED->Start();
  }
  strip->Show();
}

CubicGasSensors cubicCo2(onCo2Status, EE_RESET_CO2_1B);

void sendCO2Value() {
  int val = cubicCo2.getCO2(DEBUG);
  String s = String("sndiot ") + val;
  sndIOT(s.c_str());
  lastSentCO2value = val;
  tmrCO2SendValueTimer->Start();
}

void onCO2RawRead() {
  int res = cubicCo2.getCO2(DEBUG);
  startedCO2Monitoring = cubicCo2.hasStarted();
  if (startedCO2Monitoring) {
    int diff = res - lastSentCO2value;
    if ((co2Threshold > 0) && (abs(diff) > co2Threshold)) {
      if (DEBUG) Serial << F("Threshold reached, sending value") << endl;
      sendCO2Value();
    }
  }
}

void initCO2Handler() {

  char tmp[20];

  if (PropertyList.hasProperty(PROP_SND_INT)) intCO2SendValue = PropertyList.readLongProperty(PROP_SND_INT)*1000;
  if (PropertyList.hasProperty(PROP_SND_THR)) co2Threshold    = PropertyList.readLongProperty(PROP_SND_THR);
  Serial << F("Send Interval (ms): ") << intCO2SendValue << F(", Threshold (ppm): ") << co2Threshold << endl;

  tmrStopLED           = new Timer(30000L, onStopLED, true);
  tmrCO2RawRead        = new Timer(intCO2RawRead,   onCO2RawRead);
  tmrCO2SendValueTimer = new Timer(intCO2SendValue, sendCO2Value);
  cubicCo2.init();
  SERIAL << "CO2 now: " << cubicCo2.rawReadCM1106_CO2() << endl;
  tmrCO2RawRead->Start();

}

void loopCO2Handler() {
  tmrCO2RawRead->Update();
  tmrCO2SendValueTimer->Update();
  tmrStopLED->Update();
  delay(3000);
}

void resetCO2(const char *ignore) {
  SERIAL << F("Calibration Mode Enabled.\nPlease put the device for 5 minutes at fresh air.\nYou can now put it outside. It will complete calibration once it worked 5 minutes after restart") << endl;
  EEPROM.begin(10);
  EEPROM.put(EE_RESET_CO2_1B, (byte)1);
  EEPROM.commit();
  ESP.restart();
}

void setSendInterval (const char *line) {
  int interval = 120;
  if (strchr(line, ' ')) {
    interval = atoi(strchr(line, ' ') + 1);
  }
  PropertyList.putProperty(PROP_SND_INT, String(interval).c_str());
  intCO2SendValue = (uint32_t)interval * 1000;
  tmrCO2SendValueTimer->setInterval(intCO2SendValue);
  Serial << "Send Interval (ms): " << intCO2SendValue << endl;
}

void setSendThreshold(const char *line) {
  int thr = 0;
  if (strchr(line, ' ')) {
    thr = atoi(strchr(line, ' ') + 1);
  }
  PropertyList.putProperty(PROP_SND_THR, String(thr).c_str());
//  putJSONConfig(XX_SND_THR, String(thr).c_str());
  co2Threshold = thr;
  Serial << F("CO2 Threshold (ppm): ") << co2Threshold << endl;
}

void CO2_registerCommands(MenuHandler *handler) {
  handler->registerCommand(new MenuEntry(F("wsi"), CMD_BEGIN, setSendInterval , F("")));
  handler->registerCommand(new MenuEntry(F("wst"), CMD_BEGIN, setSendThreshold, F("")));
  handler->registerCommand(new MenuEntry(F("rco"), CMD_EXACT, resetCO2, F("")));
  // else if (strstr(line, "wsi ")) setSendInterval (line);
  // else if (strstr(line, "wst ")) setSendThreshold(line);
  // else if (strcmp(line, "rco") == 0) resetCO2();
}

//#endif
