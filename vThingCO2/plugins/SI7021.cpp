#ifdef VTHING_STARTER
#include "common.hpp"
#include <Si7021.h>
#include <Wire.h>

SI7021 *si7021;
boolean si7021init() {
    si7021 = new SI7021();
    si7021->begin(D1, D6); // Runs : Wire.begin() + reset()
    if (si7021->getDeviceID() == 255) {
      delete si7021;
      si7021 = NULL;
      return false;
    }
    SERIAL << "Found SI7021 Temperature/Humidity Sensor\n" << endl;
    si7021->setHumidityRes(8); // Humidity = 12-bit / Temperature = 14-bit
    return true;
}

void onTempRead() {
  if (!si7021) return;
  float tmp = si7021->readTemp();
  float hum = si7021->readHumidity();
  SERIAL << F("Humidity : ") << hum << " %\t";
  SERIAL << F("Temp : "    ) << tmp << " C" << endl;

  String s = String("sndiot ") + tmp;
  sndIOT(s.c_str());
}

void dumpTemp() {
  if (!si7021) return;
  SERIAL << F("Temp : ")     << si7021->readTemp() << " C" << endl;
}

#endif
