#ifdef VTHING_STARTER

#include <Si7021.h>
SI7021 *si7021;
void si7021init() {
    si7021 = new SI7021();
    si7021->begin(D1, D6); // Runs : Wire.begin() + reset()
    si7021->setHumidityRes(8); // Humidity = 12-bit / Temperature = 14-bit
}
void onTempRead() {
  float tmp = si7021->readTemp();
  float hum = si7021->readHumidity();
  SERIAL << F("Humidity : ") << hum << " %\t";
  SERIAL << F("Temp : "    ) << tmp << " C" << endl;
  
  String s = String("sndiot ") + tmp;
  sndIOT(s.c_str());
}

void dumpTemp() {
  SERIAL << F("Temp : ")     << si7021->readTemp() << " C" << endl;  
}

#endif
