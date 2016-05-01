#ifdef VTHING_CO2
#include <CubicGasSensors.h>

CubicGasSensors cubicCo2(D5, D7, EE_1B_RESET_CO2);

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
      Serial << F("Threshold reached, sending value") << endl;
      sendCO2Value();
    }    
  }
}

void initCO2Handler() {
  strip = new NeoPixelBus<NeoGrbFeature, NeoEsp8266Uart800KbpsMethod> (1, D4);
  if (strip) {
    strip->Begin();
    strip->SetPixelColor(0, RgbColor(0, 30,0));
    strip->Show();  
  }
  if (getJSONConfig(XX_SND_INT)) intCO2SendValue = getJSONConfig(XX_SND_INT).toInt() *1000;
  if (getJSONConfig(XX_SND_THR)) co2Threshold    = getJSONConfig(XX_SND_THR).toInt();
  Serial << F("Send Interval (ms): ") << intCO2SendValue << F(", Threshold (ppm): ") << co2Threshold << endl;
  
  tmrStopLED           = new Timer(10000, onStopLED, true);
  tmrCO2RawRead        = new Timer(intCO2RawRead,   onCO2RawRead);
  tmrCO2SendValueTimer = new Timer(intCO2SendValue, sendCO2Value);
  SERIAL << "CO2 now: " << cubicCo2.rawReadCM1106_CO2() << endl;
  tmrCO2RawRead->Start();
  tmrStopLED->Start();
}

void loopCO2Handler() {
  tmrCO2RawRead->Update();
  tmrCO2SendValueTimer->Update();
  tmrStopLED->Update();
  delay(5000);
}

void resetCO2() {
  SERIAL << F("Calibration Mode Enabled./nPlease put the device for 5 minutes at fresh air./nYou can safely turn it off. It will continue calibration after restart") << endl;
  EEPROM.put(EE_1B_RESET_CO2, (byte)1);
}

#endif
