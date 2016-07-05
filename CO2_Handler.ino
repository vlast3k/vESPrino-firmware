#ifdef VTHING_CO2
#include <CubicGasSensors.h>

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

CubicGasSensors cubicCo2(onCo2Status, EE_1B_RESET_CO2);

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
  if (getJSONConfig(XX_SND_INT, tmp)[0]) intCO2SendValue = String(getJSONConfig(XX_SND_INT)).toInt() *1000;
  if (getJSONConfig(XX_SND_THR, tmp)[0]) co2Threshold    = String(getJSONConfig(XX_SND_THR)).toInt();
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

void resetCO2() {
  SERIAL << F("Calibration Mode Enabled.\nPlease put the device for 5 minutes at fresh air.\nYou can now put it outside. It will complete calibration once it worked 5 minutes after restart") << endl;
  EEPROM.put(EE_1B_RESET_CO2, (byte)1);
  EEPROM.commit();
  ESP.restart();
}

#endif
